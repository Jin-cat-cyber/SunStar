#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D uDepth;       // hdrDepthTex：窗口深度（非线性，[0,1]）
uniform mat4  uInvVP;           // 本帧 VP 的逆（实体支路：把像素反算成世界坐标）
uniform mat4  uVPPrev;          // 上一帧 VP（实体支路：世界坐标投到上一帧屏幕）
uniform mat4  uProjPrev;        // 上一帧投影（天空支路：方向投到上一帧屏幕）
uniform mat3  uOrientCur;       // 本帧朝向矩阵 R（列为 Right / Up / -Front，世界空间）
uniform mat3  uSkyRot;          // R_prev * R_cur^T（天空支路：把世界方向转到上一帧的朝向）
uniform float uTanHalfFov;      // 本帧实际用的 tan(fov/2)，含 FOV 冲击
uniform float uAspect;          // 宽高比

void main()
{
    float d = texture(uDepth, TexCoords).r;
    vec2 uvPrev;

    if (d >= 1.0)
    {
        //  天空支路（计划书 §6.3）：这些像素深度恰为 1，反算世界坐标没有意义（点在无穷远）。
        //  天空只随朝向变，所以按式 (21) 把视线方向转到上一帧的朝向，再用上一帧投影投出去。
        vec2 ndc = TexCoords * 2.0 - 1.0;
        vec3 rc = vec3(ndc.x * uAspect * uTanHalfFov, ndc.y * uTanHalfFov, -1.0);  // 相机空间射线
        vec3 dirCur = normalize(uOrientCur * rc);                                   // 世界方向
        vec3 dirPrev = uSkyRot * dirCur;                                            // 上一帧的世界方向
        vec4 c = uProjPrev * vec4(dirPrev, 1.0);
        uvPrev = (c.xy / c.w) * 0.5 + 0.5;
    }
    else
    {
        //  实体支路：深度反算世界坐标（式 18），再用上一帧 VP 投到上一帧屏幕（式 19）。
        vec4 world = uInvVP * vec4(TexCoords * 2.0 - 1.0, d * 2.0 - 1.0, 1.0);
        world /= world.w;
        vec4 c = uVPPrev * vec4(world.xyz, 1.0);
        uvPrev = (c.xy / c.w) * 0.5 + 0.5;
    }

    //  【临时】自测输出：重投影误差放大 200 倍。相机静止且抖动关闭时应当全黑。
    float e = abs(uvPrev.x - TexCoords.x) + abs(uvPrev.y - TexCoords.y);
    FragColor = vec4(vec3(e * 200.0), 1.0);
}