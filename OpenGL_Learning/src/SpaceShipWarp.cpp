#include "Globals.h"

#include "SpaceShipWarp.h"

#include "Shader.h"
#include "PbrModel.h"
#include "WarpSC.h"
#include "WarpDebris.h"
#include "WarpTuning.h"
#include "WarpPillar.h"
#include "WarpShock.h"

void SpaceShipWarpInit()
{
}


void warpShellPass(Shader& warpShellShader, const glm::mat4& projection, const glm::mat4& view, const glm::mat4& spaceshipModel, const glm::vec3& camPos, float currentFrame, PbrModel& spaceship)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);      // 加法混合
    glDepthMask(GL_FALSE);            // 不写深度，别挡住后面要画的东西
    glCullFace(GL_FRONT);             // 只画背面：读起来才是"裹住"而不是"镀一层"

    warpShellShader.use();
    warpShellShader.setMat4("projection", projection);
    warpShellShader.setMat4("view", view);
    //  外壳在扫描开始时就归零（见 shellAlpha），所以它永远活不到 τ*，
    //  可以放心跟 spaceshipModel（含前冲与拉伸，贴在舰体表面上）。
    //  注意：若以后又把它的寿命拉过 τ*，就必须改用不含滑入的那一份矩阵，
    //  否则会被滑入的 −SLIDE_ARRIVAL_K 个半舰长甩在起点那一侧。
    warpShellShader.setMat4("model", spaceshipModel);   // 与 PBR pass 同一份

    warpShellShader.setVec3("camPos", camera.Position);
    warpShellShader.setVec3("uShellColor", warp_tune::U_SHELL_COLOR);
    warpShellShader.setFloat("uShellStrength", warp_sc::shellAlpha(gWarp.tau)
        * warp_tune::U_SHELL_GAIN);
    warpShellShader.setFloat("uShellWorld", shipBoundR * warp_tune::U_SHELL_WORLD_K);
    warpShellShader.setFloat("uObjRadius", shipBoundR / 0.0005f);
    warpShellShader.setFloat("uTime", currentFrame);
    warpShellShader.setFloat("uGridFreq", warp_tune::U_GRID_FREQ);       // 格子密度：26.8 / 12 ≈ 2.2 世界单位一格
    warpShellShader.setFloat("uGridWidth", warp_tune::U_GRID_WIDTH);     // 格线宽度，别超过 0.2
    warpShellShader.setFloat("uGridMix", warp_tune::U_GRID_MIX);         // 1 = 两组叠加；调 0 就退回纯正方格
    warpShellShader.setFloat("uGridStrength", warp_tune::U_GRID_STRENGTH);
    spaceship.Draw(warpShellShader);

    glCullFace(GL_BACK);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void warpGhostPass(Shader& warpGhostShader, const glm::mat4& projection, const glm::mat4& view, const glm::mat4& shipWireModel, const glm::vec3& camPos, PbrModel& spaceship)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);

    warpGhostShader.use();
    warpGhostShader.setMat4("projection", projection);
    warpGhostShader.setMat4("view", view);
    warpGhostShader.setMat4("model", shipWireModel);   // 与线框同一份：落点的最终形状
    warpGhostShader.setVec3("camPos", camera.Position);
    warpGhostShader.setVec3("uGhostColor", warp_tune::U_GHOST_COLOR);

    // 用 1 - solidify 而不是 1 - wch.a：后者挂的是梯形通道，末段会自己升回来，
    // 让幽灵舰体在收尾时重新冒出来（此前被 wch.w 的旧门控挡住，延长骨架窗口后就会露出来）。
    warpGhostShader.setFloat("uGhostStrength", warp_sc::ghostAlpha(gWarp.tau) * warp_tune::U_GHOST_GAIN);
    warpGhostShader.setFloat("uGhostExpand", shipBoundR * warp_tune::U_GHOST_EXPAND_K);
    warpGhostShader.setFloat("uAxisMinY", shipAxisMinY);
    warpGhostShader.setFloat("uAxisMaxY", shipAxisMaxY);
    warpGhostShader.setFloat("uGrow", warp_sc::wireGrow(gWarp.tau));
    warpGhostShader.setFloat("uGrowSoft", warp_tune::U_GHOST_GROW_SOFT);
    warpGhostShader.setFloat("uRimPow", warp_tune::U_GHOST_RIM_POW);
    warpGhostShader.setFloat("uBody", warp_tune::U_GHOST_BODY);

    // 抵达段的多道扫描波（7 道、周期 = 1 舰长）：速度与去程扫描对齐，
    // 也就是"一整舰长用 SCAN_TIME 秒"，于是相位直接取 (tau - TAU_STAR) / SCAN_TIME。
    // tau = TAU_STAR 时相位为 0，第 1 道正好压在艏上（与参考画面那张表的起点一致）。
    // 【现行】波列速度与去程扫描脱钩：抵达段有 4.07 秒，用 SCAN_TIME（0.60）等于跑
    // 6.8 趟，且最紧那一对（0.05 舰长）只停留 0.030 秒，观感是"闪过去"。
    // 现按参考画面读数取"整段 1.75 个舰长"，即 2.3257 s/舰长。下面那两行是旧写法，留档。
    // 速度与去程扫描对齐，
    // 也就是"一整舰长用 SCAN_TIME 秒"，于是相位直接取 (tau - TAU_STAR) / SCAN_TIME。
    // tau = TAU_STAR 时相位为 0，第 1 道正好压在艏上（与参考画面那张表的起点一致）；
    // tau = T_TOTAL 时相位正好 1.75，即刚好走完你量的那个路程。
    warpGhostShader.setFloat("uBandPhase", (gWarp.tau - warp_sc::TAU_STAR) / warp_sc::BAND_SWEEP_TIME);
    //warpGhostShader.setFloat("uBandW", warp_tune::U_GHOST_BAND_W);   // 旧：世界空间宽度
    warpGhostShader.setFloat("uBandPx", warp_tune::U_GHOST_BAND_PX);
    warpGhostShader.setVec3("uBandStrong", warp_tune::U_GHOST_BAND_STRONG);
    warpGhostShader.setVec3("uBandWeak", warp_tune::U_GHOST_BAND_WEAK);
    warpGhostShader.setFloat("uBandGate", warp_sc::wireAlpha(gWarp.tau));

    spaceship.Draw(warpGhostShader);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void warpWirePass(Shader& warpWireShader, const glm::mat4& projection, const glm::mat4& view, const glm::mat4& shipWireModel, PbrModel& spaceship)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);        // 加法混合
    glDepthMask(GL_FALSE);              // 只读深度、不写深度
    //glDisable(GL_CULL_FACE);            // 两面都画：读起来才是"全息透视线框"

    warpWireShader.use();
    warpWireShader.setMat4("projection", projection);
    warpWireShader.setMat4("view", view);
    // 线框
    //warpWireShader.setMat4("model", spaceshipModel);   // 与 PBR pass 同一份

    // 线框故意用"不拉伸"的那一份：它是舰体的最终形状（蓝图），
    // 拉伸只属于被拽出去的实体。外壳 pass 仍用拉伸后的矩阵，
    // 因为它贴在舰体表面上，必须跟着形变走。
    warpWireShader.setMat4("model", shipWireModel);

    //warpWireShader.setVec3("uWireColor", warp_tune::U_WIRE_COLOR);
    warpWireShader.setVec3("uWireColor", warp_tune::U_WIRE_COLOR_BONE);
    //warpWireShader.setFloat("uWireAlpha", wch.w);
    warpWireShader.setFloat("uWireAlpha", warp_sc::wireAlpha(gWarp.tau));   // 与蓝图同一条包络
    warpWireShader.setFloat("uWireGrow", warp_sc::wireGrow(gWarp.tau));
    warpWireShader.setFloat("uWireWidth", warp_tune::U_WIRE_WIDTH);
    warpWireShader.setFloat("uAxisMinY", shipAxisMinY);
    warpWireShader.setFloat("uAxisMaxY", shipAxisMaxY);
    spaceship.Draw(warpWireShader);

    //glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void warpPillarPass(Shader& warpDebrisShader, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& camPos, const glm::mat4& pillarPlane)
{
	WarpPillarDraw(warpDebrisShader, projection, view, camPos, shipBoundR, pillarPlane);
}


void warpDebrisPass(Shader& warpDebrisShader, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& camPos)
{
	WarpDebrisDraw(warpDebrisShader, projection, view, camPos, shipBoundR);
}


void warpShockPass(Shader& warpShockShader, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& camPos, const glm::mat4& axisModel)
{
	//  末位 true = 透明环（alpha 混合）。改回 false 就回到加法版，一行的事。
	WarpShockDraw(warpShockShader, projection, view, camPos, shipBoundR, axisModel, true);
}


void warpShockDistortPass(Shader& warpShockDistortShader, const glm::mat4& projection, const glm::mat4& view, const glm::mat4& axisModel, int windowwidth, int windowheight, unsigned int quadVAO)
{
	WarpShockDistort(warpShockDistortShader, projection, view, axisModel, windowwidth, windowheight, quadVAO);
}


void warpFlashPass(Shader& warpFlashShader, float flashAmount, float rejectFlash, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& shipPos, unsigned int quadVAO)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);        // 加法混合
    glDisable(GL_DEPTH_TEST);           // 全屏 quad 不需要深度
    glDepthMask(GL_FALSE);

    // 舰体的屏幕位置（NDC），作为 bloom 的圆心。直接投影舰体位置，
    // 比用 gl_FragCoord 反算省一个分辨率 uniform；clip.w 太小时（点在
    // 相机后面）保持 (0,0)，宁可圆心偏掉也不要除出 NaN。
    glm::vec2 flashCenter(0.0f);
    const glm::vec4 clip = projection * view * glm::vec4(shipPos, 1.0f);
    if (clip.w > 1e-4f) flashCenter = glm::vec2(clip.x / clip.w, clip.y / clip.w);

    //warpFlashShader.use();
    //warpFlashShader.setFloat("uFlash", flashAmount);
    //warpFlashShader.setFloat("uFlashBase", 6.0f);
    //warpFlashShader.setFloat("uFlashCore", 10.0f);
    //warpFlashShader.setVec2("uFlashCenter", flashCenter);
    // 两者互斥：拒绝只发生在"不在折跃、不在冷却"时，白闪只在折跃期间。
    const bool red = (rejectFlash > 0.0f);
    warpFlashShader.use();
    warpFlashShader.setFloat("uFlash", red ? rejectFlash : flashAmount);
    warpFlashShader.setFloat("uFlashBase", red ? warp_tune::U_FLASH_REJECT_BASE : warp_tune::U_FLASH_BASE);
    warpFlashShader.setFloat("uFlashCore", red ? warp_tune::U_FLASH_REJECT_CORE : warp_tune::U_FLASH_CORE);
    warpFlashShader.setVec3("uFlashColor", red ? warp_tune::U_FLASH_REJECT_COLOR : warp_tune::U_FLASH_COLOR);
    warpFlashShader.setVec2("uFlashCenter", flashCenter);


    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}