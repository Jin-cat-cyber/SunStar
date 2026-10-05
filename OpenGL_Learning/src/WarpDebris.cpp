#include "Globals.h"          // 里面第一行就是 glad，所以 glad 一定排在其它头之前
#include "WarpDebris.h"
#include "Shader.h"
#include "WarpTuning.h"
#include "WarpPillar.h"
#include "WarpShock.h"

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
    
    unsigned int shockVAO = 0, shockVBO = 0;

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

    //  单位圆盘（半径 1，躺在 XZ 平面里，法线恒为 +Y）：三角扇。
    //  "环"不是靠网格做出来的，而是靠片元按半径把带以外的部分丢掉 ——
    //  这样环带的【世界宽度】可以恒定，不会随半径一起放大。
    //  法线取 +Y：锚点的 Y 轴就是舰体纵轴，所以"环垂直于纵轴"由构造成立。
    constexpr int kShockSegs = 128;
    constexpr int kShockVerts = kShockSegs * 3;

    void BuildRing(float* out)
    {
        int w = 0;
        for (int i = 0; i < kShockSegs; ++i)
        {
            const float a0 = 6.2831853f * i / kShockSegs;
            const float a1 = 6.2831853f * (i + 1) / kShockSegs;
            const float px[3] = { 0.0f, std::cos(a0), std::cos(a1) };
            const float pz[3] = { 0.0f, std::sin(a0), std::sin(a1) };

            for (int k = 0; k < 3; ++k)
            {
                out[w++] = px[k]; out[w++] = 0.0f; out[w++] = pz[k];   // 位置：Y 恒为 0
                out[w++] = 0.0f;  out[w++] = 1.0f; out[w++] = 0.0f;   // 法线：+Y（就是纵轴）
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

    //  冲击波的圆盘：与碎屑共用顶点布局（位置 + 法线，stride 6），只是非实例化。
    float ring[kShockVerts * 6];
    BuildRing(ring);
    glGenVertexArrays(1, &shockVAO);
    glGenBuffers(1, &shockVBO);
    glBindVertexArray(shockVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shockVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ring), ring, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
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
            //const float alpha = warp_sc::pillarAlpha(p, gWarp.tau);
            const float alpha = arrival ? warp_sc::pillarArriveAlpha(p, gWarp.tau)
                : warp_sc::pillarAlpha(p, gWarp.tau);
           
            if (alpha <= 0.002f) continue;

            //const float phase = warp_sc::pillarPhase(p, gWarp.tau);
            //const float shrink = warp_sc::pillarShrinkScale(phase);   // 末尾缩掉的倍率。
            //const float len = p.half[1] * warp_sc::pillarLengthScale(phase) * shrink;
            //if (len <= 1e-4f) continue;

            const float phase = warp_sc::pillarPhase(p, gWarp.tau);
            //  长度与粗细：
            //    去程 —— 长度按【自身寿命比例】生长、末段缩掉；粗细跟着同一个缩掉倍率。
            //    抵达 —— 三者都按【全局时间表】：出生后极短时间长满，之后长度与横截面
            //            【一起】变小，所以整根始终保持长宽比，到最后也是一个小长方体，
            //            而不是被压成一张薄片。到 P_ARRIVE_T1 归零。
            float len, wide;
            if (arrival)
            {
                const float g = warp_sc::pillarArriveGrow(gWarp.tau, p.t0);
                const float sc = warp_sc::pillarArriveLenScale(gWarp.tau);
                len = p.half[1] * g * sc;
                //wide = g * sc;
                wide = g * std::pow(sc, warp_sc::P_ARRIVE_WIDE_P);
            }
            else
            {
                const float sc = warp_sc::pillarShrinkScale(phase);
                len = p.half[1] * warp_sc::pillarLengthScale(phase) * sc;
                wide = sc;
            }
            if (len <= 1e-4f) continue;

            //  内端钉在出生点上、朝 dirY 一侧伸长，所以中心要跟着当前长度走（计划书命题 3）。
            //const float s = warp_sc::pillarDrift(phase);
            //const float s = arrival ? warp_sc::pillarDriftArrive(phase) : warp_sc::pillarDrift(phase);
            
            //  两段都是线性：抵达要恒定前飘，去程现在也要匀速（配合上面的速度制）。
            //  disp 是"速度 × 寿命"，乘上线性的 phase 之后，速度恰好与时间无关。
            const float s = phase;
            const glm::vec3 pos(
                p.p0[0] + p.disp[0] * s,
                p.p0[1] + p.disp[1] * s + p.dirY * len,
                p.p0[2] + p.disp[2] * s);

            //  与碎屑一样：实例矩阵只含平移（柱子姿态与舰体局部轴对齐），缩放走 aHalf。
            const glm::mat4 inst = glm::translate(glm::mat4(1.0f), pos);

            float* o = &debrisInstanceData[static_cast<size_t>(count) * kInstanceFloats];
            const float* m = &inst[0][0];
            for (int k = 0; k < 16; ++k) o[k] = m[k];
            o[16] = p.half[0] * wide; o[17] = len; o[18] = p.half[2] * wide;
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


// ===== GL 侧：冲击波（两道波共用这一支）=================================
//  锚点矩阵的 Y 轴就是波的中心轴、原点就是波的原点：
//    去程收束波传 shipWireModel；抵达发散波传光柱那张面（pillarPlane）。
//  与碎屑/光柱同样的尺度还原：锚点里含 scale(0.0005)，而本模块坐标按世界单位写。
void WarpShockDraw(Shader& shader, const glm::mat4& projection, const glm::mat4& view,
    const glm::vec3& camPos, float shipBoundR, const glm::mat4& axisModel)
{
    if (!gWarp.active) return;

    const bool out = warp_sc::shockOutLive(gWarp.tau);
    const bool in = warp_sc::shockInLive(gWarp.tau);
    if (!out && !in) return;                 // 两道窗口互不相交，这一帧没有波

    const float r = out ? warp_sc::shockRadiusOut(gWarp.tau)
        : warp_sc::shockRadiusIn(gWarp.tau);
    const float energy = warp_sc::shockEnergy(r);        // 平面圆环：二维圆波，I ∝ 1/r
    const float env = out ? warp_sc::shockWindowOut(gWarp.tau)
        : warp_sc::shockWindowIn(gWarp.tau);   // 窗口包络：两端归零

    const glm::mat4 anchor = axisModel * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / 0.0005f));
    //  圆盘躺在锚点的 XZ 平面里、法线是其 Y 轴（= 舰体纵轴）。
    //  【现行】缩放用【环半径 + 2.5 个带宽】。系数不能小于 sqrt(ln(1/0.002)) = 2.4929：
    //  片元的丢弃阈值是 band <= 0.002；网格外缘若比它更靠内，最外一圈还留着约 1.8% 的亮度，
    //  而它外面什么都没有，圆盘边缘就会读出一条硬台阶（旧的 2.0 正是如此）。
    //  以下两行是高斯环带时期的旧说法，数值已过时，留档对比：
    //  缩放用的是【环半径 + 2 个带宽】而不是环半径：环带以 r 为中心、内外各 uBandW，
    //  网格若正好只到 r，外侧那半边就没有像素可画，圆的边缘会变成一刀切的硬边。
    
    //const float ringScale = r + 2.0f * warp_sc::S_BAND_W;       // 
    const float ringScale = r + 2.5f * warp_sc::S_BAND_W;       // 更柔和
    const glm::mat4 model = anchor * glm::scale(glm::mat4(1.0f), glm::vec3(ringScale, 1.0f, ringScale));

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);

    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setMat4("model", model);
    shader.setVec3("camPos", camPos);
    shader.setVec3("uDebrisColor", warp_tune::U_PILLAR_COLOR);
    //shader.setVec3("uShockTint", warp_tune::U_SHOCK_TINT);    // 不设就是 (0,0,0) —— 波会全黑
    
        //  行进色用【窗口进度】而不是半径：今天两者恒等（S_OUT_R0 与 S_REF_R 都是 60），
    //  但以后改任何一侧的半径都不会再连累配色 —— 半径是"走多远"，进度是"走到哪一刻"。
    //  顺带让 S_REF_R 回到只管能量律一条语义。
    const float colorP = out ? warp_sc::shockProgress(gWarp.tau, warp_sc::S_OUT_T0, warp_sc::S_OUT_T1)
        : warp_sc::shockProgress(gWarp.tau, warp_sc::S_IN_T0, warp_sc::S_IN_T1);
    const float colorK = out ? (1.0f - colorP) : colorP;

    shader.setVec3("uShockTint",
        glm::mix(warp_tune::U_SHOCK_TINT_NEAR, warp_tune::U_SHOCK_TINT_FAR, colorK));
    
    shader.setFloat("uDebrisStrength", warp_tune::U_SHOCK_GAIN * energy * env);
    shader.setFloat("uRingR", r);
    shader.setFloat("uRingScale", ringScale);   // 网格的世界外半径，片元用它把物体半径换算成世界半径
    shader.setFloat("uBandW", warp_sc::S_BAND_W);
    shader.setFloat("uFacingFloor", warp_sc::S_FACING_FLOOR);

    glBindVertexArray(shockVAO);
    glDrawArrays(GL_TRIANGLES, 0, kShockVerts);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


// ===== GL 侧：冲击波的屏幕空间扭曲 ======================================
//  【现行】逐像素用深度反算世界坐标，算出它到【那圈圆环】的距离（面外分量也算在内，
//  不是到纵轴的垂直距离），再按"离波前多远"给一个单峰的屏幕空间径向偏移。判定必须在世界
//  空间做：环是躺在一张平面里的圆，投到屏幕上一般不是圆（只有正对时才是）。
//  以下两行是圆柱壳 / 球壳时期的措辞，"到波轴的垂直距离"与"圆柱"两处已过时，
//  结论（用世界距离而不是屏幕距离）不变，留档对比：
//  逐像素用深度反算世界坐标，算出它到波轴的垂直距离，再按"离波前多远"给一个单峰的
//  屏幕空间径向偏移。用【世界距离】而不是屏幕距离 —— 圆柱投到屏幕上不是圆，相机一转就错。
//  渲染到 pingpongFBO[0]（与 hdrColorBuffer 同为 RGBA16F），再 blit 回 hdrColorBuffer。
void WarpShockDistort(Shader& shader, const glm::mat4& projection, const glm::mat4& view,
    const glm::mat4& axisModel, int screenW, int screenH, unsigned int quadVAO)
{
    if (!gWarp.active) return;

    const bool out = warp_sc::shockOutLive(gWarp.tau);
    const bool in = warp_sc::shockInLive(gWarp.tau);
    if (!out && !in) return;

    const float r = out ? warp_sc::shockRadiusOut(gWarp.tau) : warp_sc::shockRadiusIn(gWarp.tau);
    const float push = out ? warp_sc::S_OUT_PUSH : warp_sc::S_IN_PUSH;

    //  【现行】扭曲自己的包络，两端都管：起点与【该道波自己的】亮带淡入同源（避免"还没见光、背景先扭"），
    //  终点用自己那条收尾占比 S_DIST_FALL_F，两端精确归零。推导见计划书 §9.4 式 (23)。
    //  以下那行只说了收尾，是"只有收尾"那一版的措辞，留档对比：
    //  扭曲自己的收尾（与亮度的包络不同：更晚、更短），避免在窗口边界一帧剪断。
    /*const float ramp = out
        ? warp_sc::shockDistortRamp(gWarp.tau, warp_sc::S_OUT_T0, warp_sc::S_OUT_T1)
        : warp_sc::shockDistortRamp(gWarp.tau, warp_sc::S_IN_T0, warp_sc::S_IN_T1);*/

    const float ramp = out
        ? warp_sc::shockDistortRamp(gWarp.tau, warp_sc::S_OUT_T0, warp_sc::S_OUT_T1, warp_sc::S_OUT_RISE_F)
        : warp_sc::shockDistortRamp(gWarp.tau, warp_sc::S_IN_T0, warp_sc::S_IN_T1, warp_sc::S_IN_RISE_F);

    //  【现行】环心 = 锚点矩阵的平移列（环躺在垂直于纵轴的平面里，圆心落在纵轴上）。
    //  以下两行里的"球心"是球壳时期的说法，坐标取法的结论不变，留档对比：
    //  球心 = 锚点矩阵的平移列。锚点里那个 scale(0.0005) 只影响线性部分，
    //  平移列（第 4 列）本来就是世界单位，所以直接用即可。
    const glm::vec3 center(axisModel[3]);
    const glm::vec3 axisDir = glm::normalize(glm::vec3(axisModel[1]));   // 环所在平面的法线 = 舰体纵轴

    const glm::mat4 vp = projection * view;
    const glm::mat4 invVP = glm::inverse(vp);

    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    shader.setInt("uScene", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, hdrDepthTex);
    shader.setInt("uDepth", 1);
    shader.setMat4("uVP", vp);
    shader.setMat4("uInvVP", invVP);
    shader.setVec3("uCenter", center);
    shader.setVec3("uAxisDir", axisDir);
    shader.setFloat("uRadius", r);
    shader.setFloat("uProfileW", warp_sc::S_PROFILE_W);
    //shader.setFloat("uDistortPx", warp_sc::S_DISTORT_PX);
    shader.setFloat("uDistortPx", warp_sc::S_DISTORT_PX * ramp);
    shader.setFloat("uPush", push);
    shader.setVec2("uScreenSize",
        glm::vec2(static_cast<float>(screenW), static_cast<float>(screenH)));

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    //  搬回 hdrColorBuffer。pingpongFBO[0] 随后会被 PostProcessing 的亮度提取重新覆盖，不冲突。
    glBindFramebuffer(GL_READ_FRAMEBUFFER, pingpongFBO[0]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO);
    glBlitFramebuffer(0, 0, screenW, screenH, 0, 0, screenW, screenH,
        GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
}