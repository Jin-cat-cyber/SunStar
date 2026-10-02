#include "Globals.h"          // 里面第一行就是 glad，所以 glad 一定排在其它头之前
#include "WarpDebris.h"
#include "Shader.h"
#include "WarpTuning.h"
#include "WarpPillar.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

// ---------------------------------------------------------------------------
//  WarpDebris.cpp —— SC 分支起飞段碎屑的 GL 侧：立方体网格、实例缓冲与绘制。
//
//  纯运动学在 WarpDebris.h 的 namespace warp_sc 里（header-only、不含 GL），
//  这个文件只做三件事：建网格与缓冲、每帧记一次锚点姿态、把实例矩阵填好画出去。
//  缓冲与锚点都是模块私有（匿名命名空间），所以 Globals.h 里不再需要它们。
// ---------------------------------------------------------------------------

namespace
{
    // 每实例 23 个浮点：mat4（16）+ 半尺寸（3）+ 色偏与强度（4）。
    constexpr int kInstanceFloats = 23;

    unsigned int debrisVAO = 0, debrisVBO = 0, debrisInstanceVBO = 0;
    
    // 24.0 版本光柱
    //std::vector<float> debrisInstanceData(static_cast<size_t>(warp_sc::D_COUNT)* kInstanceFloats, 0.0f);
    
    
    // 碎屑与光柱各自一次填、一次画，本可只取两者的较大者；这里按"碎屑 + 两段光柱窗口"开，
    // 留出以后把窗口改重叠的余量。
    std::vector<float> debrisInstanceData(
        static_cast<size_t>(warp_sc::D_COUNT + 2 * warp_sc::P_COUNT)* kInstanceFloats, 0.0f);

    // 锚点：折跃开始那一帧的舰体姿态（已还原到世界尺度）。碎屑是被"甩掉"的，
    // 不该跟着舰体继续飞，所以整趟折跃只用这一份冻结的姿态。
    glm::mat4 debrisAnchor(1.0f);
    bool debrisAnchorValid = false;

    // 程序化生成一份立方体：6 面 × 2 三角形 × 3 顶点，每顶点 6 个浮点（位置 + 法线）。
    // 顶点取 ±1 而不是 ±0.5，于是实例里的半尺寸直接就是世界单位的半边长，
    // 压扁某一轴就得到矩形板 —— 方块与矩形共用这一份网格。
    // u、v 取 (axis+1)%3 与 (axis+2)%3，恒与 axis 互异，三个坐标保证落在三个不同轴上；
    // 绕序无论正反都不影响观感 —— 本工程从不启用面剔除。
    void BuildCube(float* out)
    {
        const float corner[4][2] = { {-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f} };
        const int   tri[6] = { 0, 1, 2, 0, 2, 3 };
        int w = 0;
        for (int axis = 0; axis < 3; ++axis)
        {
            const int u = (axis + 1) % 3;
            const int v = (axis + 2) % 3;
            for (int sgn = 0; sgn < 2; ++sgn)
            {
                const float s = (sgn == 0) ? -1.0f : 1.0f;
                for (int k = 0; k < 6; ++k)
                {
                    float p[3] = { 0.0f, 0.0f, 0.0f };
                    p[axis] = s;
                    p[u] = corner[tri[k]][0];
                    p[v] = corner[tri[k]][1];
                    out[w++] = p[0]; out[w++] = p[1]; out[w++] = p[2];
                    out[w++] = (axis == 0) ? s : 0.0f;
                    out[w++] = (axis == 1) ? s : 0.0f;
                    out[w++] = (axis == 2) ? s : 0.0f;
                }
            }
        }
    }
}

void WarpDebrisInit()
{
    float cube[36 * 6];
    BuildCube(cube);

    glGenVertexArrays(1, &debrisVAO);
    glGenBuffers(1, &debrisVBO);
    glGenBuffers(1, &debrisInstanceVBO);

    glBindVertexArray(debrisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debrisVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    // 实例属性必须在这个 VAO 绑定期间挂上去，除数设 1。
    glBindBuffer(GL_ARRAY_BUFFER, debrisInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(debrisInstanceData.size() * sizeof(float)), nullptr, GL_DYNAMIC_DRAW);
    for (int k = 0; k < 4; ++k)
    {
        glEnableVertexAttribArray(2 + k);
        glVertexAttribPointer(2 + k, 4, GL_FLOAT, GL_FALSE,
            kInstanceFloats * sizeof(float), (void*)(k * 4 * sizeof(float)));
        glVertexAttribDivisor(2 + k, 1);
    }
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, kInstanceFloats * sizeof(float), (void*)(16 * sizeof(float)));
    glVertexAttribDivisor(6, 1);
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, kInstanceFloats * sizeof(float), (void*)(19 * sizeof(float)));
    glVertexAttribDivisor(7, 1);
    glBindVertexArray(0);
}

void WarpDebrisAnchor(const glm::mat4& shipModel)
{
    if (!gWarp.active) { debrisAnchorValid = false; return; }
    if (debrisAnchorValid) return;

    // 乘 scale(1/0.0005) 把物体空间还原到世界尺度，于是碎片坐标可以直接按世界单位写。
    debrisAnchor = shipModel * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / 0.0005f));
    debrisAnchorValid = true;
}

void WarpDebrisDraw(Shader& shader, const glm::mat4& projection, const glm::mat4& view,
    const glm::vec3& camPos, float shipBoundR)
{
    // 碎屑活过传送（D_END = 1.95 > tau* = 1.10），因为它们留在原地：
    // 跟随视角下已经跟着舰体走了、看不到，自由/远程视角下才看得到原地残留的碎片。
    if (!gWarp.active || gWarp.tau >= warp_sc::D_END) return;

    int count = 0;
    for (int i = 0; i < warp_sc::D_COUNT; ++i)
    {
        const warp_sc::Debris d = warp_sc::debrisAt(i, shipBoundR);
        const float alpha = warp_sc::debrisAlpha(d, gWarp.tau);
        if (alpha <= 0.002f) continue;

        // 位移 = v * life * b(相位)：b(0)=0, b'(0)=2, b(1)=1, b'(1)=0，
        // 所以初速非零（是被甩出去的）、末速为零（不会一直飞）。
        const float s = warp_sc::debrisBurst(warp_sc::debrisPhase(d, gWarp.tau)) * d.life;
        const glm::vec3 pos(d.p0[0] + d.v[0] * s, d.p0[1] + d.v[1] * s, d.p0[2] + d.v[2] * s);
        
        //const glm::vec3 axis(d.axis[0], d.axis[1], d.axis[2]);

        //// 实例矩阵只含旋转与平移（缩放走 aHalf），法线才是精确变换。
        //// 自转从出生那一刻起算，位置与姿态在 t0 处一起出现。
        //const glm::mat4 inst = glm::translate(glm::mat4(1.0f), pos)
        //    * glm::rotate(glm::mat4(1.0f), d.spin * std::max(0.0f, gWarp.tau - d.t0), axis);

        // 碎片不自转：姿态与舰体局部轴对齐（见 WarpDebris.h 的 debrisAt），
        // 所以实例矩阵只剩平移，缩放仍然单独走 aHalf。
        const glm::mat4 inst = glm::translate(glm::mat4(1.0f), pos);

        float* o = &debrisInstanceData[static_cast<size_t>(count) * kInstanceFloats];
        const float* m = &inst[0][0];
        for (int k = 0; k < 16; ++k) o[k] = m[k];
        o[16] = d.half[0]; o[17] = d.half[1]; o[18] = d.half[2];
        o[19] = 0.55f + 0.45f * d.tint;   // 色偏：偏青到偏白
        o[20] = 0.60f + 0.40f * d.tint;
        o[21] = 0.70f + 0.30f * d.tint;
        o[22] = alpha;
        ++count;
    }
    if (count == 0) return;             // 这一帧没有碎片，连 GL 状态都不碰

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);

    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setMat4("model", debrisAnchor);
    shader.setVec3("camPos", camPos);
    shader.setVec3("uDebrisColor", warp_tune::U_DEBRIS_COLOR);
    shader.setFloat("uDebrisStrength", warp_tune::U_DEBRIS_STRENGTH);
    shader.setFloat("uDebrisEdge", warp_tune::U_DEBRIS_EDGE);

    glBindVertexArray(debrisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debrisInstanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
        static_cast<GLsizeiptr>(count * kInstanceFloats * sizeof(float)), debrisInstanceData.data());
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, count);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void WarpPillarDraw(Shader& shader, const glm::mat4& projection, const glm::mat4& view,
    const glm::vec3& camPos, float shipBoundR, const glm::mat4& logicalModel)
{
    if (!gWarp.active || gWarp.tau >= warp_sc::P_END) return;

    int count = 0;
    for (int w = 0; w < 2; ++w)
    {
        const bool arrival = (w == 1);
        if (!arrival && !warp_sc::P_DEPART_ENABLE) continue;   // 去程这一轮先不接线。
        if (!warp_sc::pillarWindowLive(gWarp.tau, arrival)) continue;

        const int pillarCount = arrival ? warp_sc::P_COUNT : warp_sc::P_DEPART_COUNT;
        for (int i = 0; i < pillarCount; ++i)
        {
            const warp_sc::Pillar p = warp_sc::pillarAt(i, shipBoundR, arrival);
            const float alpha = warp_sc::pillarAlpha(p, gWarp.tau);
            if (alpha <= 0.002f) continue;

            //const float phase = warp_sc::pillarPhase(p, gWarp.tau);
            //const float shrink = warp_sc::pillarShrinkScale(phase);   // 末尾缩掉的倍率。
            //const float len = p.half[1] * warp_sc::pillarLengthScale(phase) * shrink;
            //if (len <= 1e-4f) continue;

            const float phase = warp_sc::pillarPhase(p, gWarp.tau);
            const float shrink = warp_sc::pillarShrinkScale(phase);   // 末尾缩掉的倍率。
            //  抵达段：柱长还随【出生先后】按方向变化 —— 朝折跃方向的越来越长、背向的越来越短。
            const float birthW = (p.t0 - warp_sc::P_ARRIVE_T0)
                / (warp_sc::P_ARRIVE_BIRTH_T1 - warp_sc::P_ARRIVE_T0);
            const float dirLen = arrival ? warp_sc::pillarDirLenScale(birthW, p.dirY) : 1.0f;
            const float len = p.half[1] * warp_sc::pillarLengthScale(phase) * shrink * dirLen;
            if (len <= 1e-4f) continue;

            //  内端钉在出生点上、朝 dirY 一侧伸长，所以中心要跟着当前长度走（计划书命题 3）。
            const float s = warp_sc::pillarDrift(phase);
            const glm::vec3 pos(
                p.p0[0] + p.disp[0] * s,
                p.p0[1] + p.disp[1] * s + p.dirY * len,
                p.p0[2] + p.disp[2] * s);

            //  与碎屑一样：实例矩阵只含平移（柱子姿态与舰体局部轴对齐），缩放走 aHalf。
            const glm::mat4 inst = glm::translate(glm::mat4(1.0f), pos);

            float* o = &debrisInstanceData[static_cast<size_t>(count) * kInstanceFloats];
            const float* m = &inst[0][0];
            for (int k = 0; k < 16; ++k) o[k] = m[k];
            o[16] = p.half[0] * shrink; o[17] = len; o[18] = p.half[2] * shrink;
            o[19] = 0.55f + 0.45f * p.tint;
            o[20] = 0.60f + 0.40f * p.tint;
            o[21] = 0.70f + 0.30f * p.tint;
            o[22] = alpha;
            ++count;
        }
    }
    if (count == 0) return;             // 这一帧没有柱子，连 GL 状态都不碰。

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);

    //  锚点必须先还原到世界尺度：logicalModel（shipWireModel）里含 GetModelMatrix 的
    //  scale(0.0005)，而本模块的坐标一律按世界单位写。
    //  少了这一步，整组柱子会被缩到 1/2000，变成一个亚像素的点。
    //  碎屑那边是在 WarpDebrisAnchor 里做同一件事。
    const glm::mat4 anchor =
        logicalModel * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / 0.0005f));

    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setMat4("model", anchor);
    shader.setVec3("camPos", camPos);
    shader.setVec3("uDebrisColor", warp_tune::U_PILLAR_COLOR);
    shader.setFloat("uDebrisStrength", warp_tune::U_PILLAR_GAIN);
    shader.setFloat("uDebrisEdge", warp_tune::U_PILLAR_EDGE);

    glBindVertexArray(debrisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debrisInstanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
        static_cast<GLsizeiptr>(count * kInstanceFloats * sizeof(float)), debrisInstanceData.data());
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, count);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}