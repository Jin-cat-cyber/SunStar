#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D uScene;       // hdrColorBuffer：本帧已渲染的场景（含折跃各层与扭曲）
uniform sampler2D uDepth;       // hdrDepthTex：窗口深度（非线性，[0,1]）
uniform sampler2D uHistory;     // 上一帧的累积结果（乒乓里 read 的那张）
uniform sampler2D uVelocity; // 舰体速度缓冲（a > 0.5 表示这个像素有速度可用）
uniform sampler2D uPrevDepth;   // 上一帧的场景深度（NEAREST、单独一张，不参与颜色也不被过滤）
uniform mat4  uInvVPPrev;       // 上一帧 VP 的逆：把上一帧的深度反算成世界点

uniform vec3  uCamPos;          // 本帧相机位置
uniform vec3  uCamPosPrev;      // 上一帧相机位置（去遮挡检测用）
//uniform sampler2D uRingDepth;   // 尘埃星环自己那一路深度（< 0 表示该像素没有环）
uniform mat4  uInvVP;           // 本帧 VP 的逆（实体支路反算世界坐标）
uniform mat4  uVPPrev;          // 上一帧 VP（实体支路投回上一帧屏幕）
uniform mat4  uProjPrev;        // 上一帧投影（天空支路投方向）
uniform mat3  uOrientCur;       // 本帧朝向矩阵 R（列为 Right / Up / -Front）
uniform mat3  uSkyRot;          // R_prev * R_cur^T
uniform float uTanHalfFov;      // 本帧实际用的 tan(fov/2)，含 FOV 冲击
uniform float uAspect;
uniform vec2  uScreenSize;      // 渲染分辨率（像素）：3x3 邻域的步长
uniform float uWeight;          // 当前帧权重 w（式 8）
uniform float uGamma; // variance clipping 的宽度系数（式 15）
uniform int   uHasHistory;      // 0 = 本帧无有效历史：直接输出当前帧

void main()
{
    vec3 cur = texture(uScene, TexCoords).rgb;

    float d = texture(uDepth, TexCoords).r;
    vec2 uvPrev;
    vec3 world = vec3(0.0);
    float curDist = 0.0;        // 本像素到【本帧相机】的距离（天空记 0，表示这一帧没有可比深度）
    float prevCamDist = 0.0;    // 同一个点到【上一帧相机】的距离：与历史里存的量同源，才可比

    if (d >= 1.0)
    {
        //  天空支路（§6.3）：深度恰为 1，反算世界坐标没有意义（点在无穷远）。
        //  天空只随朝向变，所以按式 (21) 把视线方向转到上一帧的朝向，再用上一帧投影投出去。
        vec2 ndc = TexCoords * 2.0 - 1.0;
        vec3 rc = vec3(ndc.x * uAspect * uTanHalfFov, ndc.y * uTanHalfFov, -1.0);
        vec3 dirCur = normalize(uOrientCur * rc);
        vec3 dirPrev = uSkyRot * dirCur;
        vec4 c = uProjPrev * vec4(dirPrev, 1.0);
        uvPrev = (c.xy / c.w) * 0.5 + 0.5;
    }
    else
    {
        //  实体支路：深度反算世界坐标（式 18），再用上一帧 VP 投到上一帧屏幕（式 19）。
        vec4 wp = uInvVP * vec4(TexCoords * 2.0 - 1.0, d * 2.0 - 1.0, 1.0);
        world = wp.xyz / wp.w;
        curDist = length(world - uCamPos);
        prevCamDist = length(world - uCamPosPrev);
        vec4 c = uVPPrev * vec4(world, 1.0);
        uvPrev = (c.xy / c.w) * 0.5 + 0.5;

        //  舰体这类"世界空间里在动的刚体"用速度缓冲覆盖相机反投影的结果：
        //  它的历史该按"这个点上一帧被画在哪"取，而不是按"相机反投影算出的位置"。
        vec4 vel = texture(uVelocity, TexCoords);
        if (vel.a > 0.5)
        {
            uvPrev = TexCoords + vel.xy;
        }
    }

    //  失效帧（首帧 / 窗口尺寸变化 / 折跃传送那一帧）直接输出当前帧，
    //  同时把本帧距离写进历史的 alpha，供下一帧做去遮挡判断。
    if (uHasHistory == 0)
    {
        FragColor = vec4(cur, curDist);
        return;
    }

    //  重投影落到画面外：这个像素的历史不存在（新露出的区域），直接输出当前帧。
    //  【现行】这一句曾一度被注释掉，现已恢复：没有它，uvPrev 跑出画面时会去采边界纹素，
    //  转镜头时屏幕四边的历史会被拉成条（就是"新露出的边缘拖糊影"那条）。
    if (uvPrev.x < 0.0 || uvPrev.x > 1.0 || uvPrev.y < 0.0 || uvPrev.y > 1.0)
    {
        FragColor = vec4(cur, curDist);
        return;
    }

    //  邻域步长：本文件后面（variance clipping 那段）也有一份同名写法，但它在下面才声明，
    //  GLSL 要求先用后声明是编译错误，所以这里用独立名字，谁都不动谁。
    vec2 pxStep = 1.0 / uScreenSize;

    //  天空侧：改成"邻域里只要还有一个纹素是天空，就承认历史"。
    //  只否掉【9 个纹素全是几何】这一种情形 —— 那才是"上一帧这个位置整片被某个物体占着、
    //  历史里存的是它的颜色"。单纹素判定会把 ±1 像素的抖动偏移也算成不一致，
    //  于是轮廓外侧那圈天空像素时不时被清一次历史，看起来就是随相机移动地颤。
    if (d >= 1.0)
    {
        bool anySky = false;
        for (int y = -1; y <= 1 && !anySky; ++y)
            for (int x = -1; x <= 1; ++x)
                if (texture(uPrevDepth, uvPrev + vec2(float(x), float(y)) * pxStep).r >= 1.0)
                {
                    anySky = true;
                    break;
                }
        if (!anySky)
        {
            FragColor = vec4(cur, curDist);
            return;
        }
    }

    //  【旧版天空判据，留档对比，已停用】单纹素版：只要重投影落点那【一个】纹素是几何就丢历史。
    //  它比上面那个 3x3 版更严，留在下面会把 3x3 的结论整个盖掉（3x3 判"邻域里有天空就放行"，
    //  紧接着它又判"落点这一点是几何就丢"），所以这一版的现行体已删除，只留形：
    //  if (d >= 1.0 && texture(uPrevDepth, uvPrev).r < 1.0) { FragColor = vec4(cur, curDist); return; }

    //  去遮挡（计划书 §9 批次 3）：把"上一帧这张深度图上、这个像素当时看到的东西有多远"取出来，
    //  与本像素的表面点在【上一帧相机】下的距离比较 —— 同一个表面时两者相等；差得超过阈值
    //  说明历史来自另一个表面（这一帧新露出来的区域），沿用它就会拖出一条。
    //  两个距离都相对【上一帧相机】量，才与上一帧的深度同源；天空（深度 = 1）没有可比深度，跳过。
    //  带速度的像素（舰体）也跳过：它是真在动，"上一帧深度"与"本帧的点"本来就该不同，速度缓冲才是它的答案。
    //  几何侧：改成"邻域里只要有一个纹素与当前表面同源（距离在容差内），就承认历史"。
    //  单纹素版（留档：落点那一个纹素的深度算出的距离超阈值就丢）在轮廓与陡坡上会误判 ——
    //  落点压在相邻纹素上时，那个纹素的深度属于别的表面或同一表面的另一侧，距离必然超阈值，
    //  于是合法历史被丢掉、这一帧退回"未累积的单次抖动采样"，症状就是随相机移动时不时颤一下。
    //  真正该丢的情形（这一帧新露出来的表面）邻域 9 个纹素一个都对不上，照样会丢。
    //  带速度的像素（舰体）仍然整个跳过：它在世界空间里真的在动，上一帧深度与它本来就不同源。
    //  注意 tol 不能写 const：GLSL 的 const 要求编译期常量，prevCamDist 是逐片元算出来的（C1059）。
    if (d < 1.0 && texture(uVelocity, TexCoords).a <= 0.5)
    {
        float tol = max(0.05 * prevCamDist, 2.0);
        bool matched = false;
        for (int y = -1; y <= 1 && !matched; ++y)
        {
            for (int x = -1; x <= 1; ++x)
            {
                vec2 uvT = uvPrev + vec2(float(x), float(y)) * pxStep;
                float pd = texture(uPrevDepth, uvT).r;
                if (pd >= 1.0) continue;
                vec4 pw = uInvVPPrev * vec4(uvT * 2.0 - 1.0, pd * 2.0 - 1.0, 1.0);
                if (abs(length(pw.xyz / pw.w - uCamPosPrev) - prevCamDist) <= tol)
                {
                    matched = true;
                    break;
                }
            }
        }
        if (!matched)
        {
            FragColor = vec4(cur, curDist);
            return;
        }
    }

    //  去遮挡：历史 alpha 里存着"上一帧这里有多远"（同样相对上一帧相机量）。
    //  差得超过阈值，说明这一帧看到的是上一帧没露出来的表面，沿用历史会拖出一条，于是取当前帧。
    //  阈值取"相对的 3% 与 1 个世界单位里的较大者"，避免远处把正常误差判成遮挡。
    
    //  【挂起】去遮挡检测：它的 alpha 会被线性过滤，而天空哨兵是 0，交界处插值出来的"上一帧距离"
    //  和真实距离差很多 → 被误判成新露出 → 历史被丢 → 静止画面出现高频抖动（尤其小物体与轮廓一带）。
    //  要恢复必须先解决"上一帧深度要 NEAREST 采样、且与颜色分开存"（见 TAA 计划书 §9 批次 3）。
//    float prevDist = texture(uHistory, uvPrev).a;
//    if (d < 1.0 && prevDist > 0.0)
//    {
//        float tol = max(0.03 * prevCamDist, 1.0);
//        if (abs(prevDist - prevCamDist) > tol)
//        {
//            FragColor = vec4(cur, curDist);
//            return;
//        }
//    }

    //  邻域 min/max 钳制（式 16）：先把历史拉回"当前帧这一小块能看到的范围"，再按 (8) 混合。
    //  钳制是有意引入的偏置（§5.3），换来的是快速错位处不留残影。
    //  variance clipping（式 14、15）：用邻域的均值 μ 与标准差 σ 当钳制区间 [μ − γσ, μ + γσ]。
    //  它比 min/max 强在"对单点异常不敏感"：min/max 会被邻域里的一个亮像素把上限撑开，
    //  在 HDR 场景里（太阳附近）几乎等于不钳制。σ 版本区间更紧，快速运动处的残影更短。
    //  代价：静态画面里的高动态范围细节会稍微不那么"稳"，γ 就是这两个方向之间的旋钮。
    //  以下是"min/max 钳制（式 16）"时期的写法，留档对比：
    //  vec3 lo = cur; vec3 hi = cur;
    //  for (...) { lo = min(lo, c3); hi = max(hi, c3); }
    vec2 texel = 1.0 / uScreenSize;
    vec3 m1 = vec3(0.0);
    vec3 m2 = vec3(0.0);
    vec3 cMax = vec3(0.0);
    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            vec3 c3 = texture(uScene, TexCoords + vec2(float(x), float(y)) * texel).rgb;
            m1 += c3;
            m2 += c3 * c3;
            cMax = max(cMax, c3);
        }
    }
    m1 /= 9.0;
    m2 /= 9.0;
    vec3 sigma = sqrt(max(m2 - m1 * m1, vec3(0.0)));
    //  亮侧取"σ 区间"与"邻域最大值"的较大者：微小高光在 3x3 里是孤立亮点，σ 很小，纯 σ 区间
    //  会把它整块夹掉 —— 那一帧的高光被压下去、下一帧又放出来，就是看到的高频抖动。
    //  取邻域最大值让它活下来，代价是亮边上的残影稍微多一点（可接受）。
    //  暗侧保持 σ 区间：那里没有"孤立暗点"这种问题，σ 区间压残影更有效。
    vec3 lo = m1 - uGamma * sigma;
    //vec3 hi = m1 + uGamma * sigma;
    vec3 hi = max(m1 + uGamma * sigma, cMax);
    vec3 hist = clamp(texture(uHistory, uvPrev).rgb, lo, hi);
    FragColor = vec4(mix(hist, cur, uWeight), curDist);
}