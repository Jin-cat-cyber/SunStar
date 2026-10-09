#version 330 core
//  SMAA 第二遍：邻域混合。第一遍标出的每一条边，在这里沿边自身的方向搜索它的两个端点；
//  只有【落在端点上的】像素需要修正 —— 那正是台阶所在。
//
//  修正量 0.5 / L（L = 这条边的长度，单位像素）：把"一个像素量级的台阶误差"分摊到 L 个像素上。
//  自检两例：L = 1（45° 斜边或孤立台阶）时两端重合，两侧各给 0.25，于是一个像素宽的细线
//  还能保住一半自身颜色；L = 4 时修正量 0.125，与"1/4 像素的台阶误差"同量级。
//
//  搜索有步数上限（uMaxSteps）：撞到上限只影响 L 的估计（偏小的一侧给得略多，量级仍很小）；
//  而"端点判定"只看紧邻的那一个像素，不受上限影响，所以长边（平缓边界，本来就看不出台阶）
//  自动不会被修正。
in vec2 TexCoords;

uniform sampler2D uColor;
uniform sampler2D uEdges;     // 第一遍的结果（RG8）
uniform vec2  uTexel;
uniform int   uMaxSteps;      // 搜索步数上限（起点 8）

out vec4 FragColor;

vec2 Edges(vec2 uv)
{
    return texture(uEdges, uv).rg;
}

void main()
{
    vec3 c  = texture(uColor, TexCoords).rgb;
    vec2 e0 = Edges(TexCoords);

    float wUp = 0.0;
    float wDn = 0.0;
    float wLf = 0.0;
    float wRt = 0.0;

    //  ---- 竖边（与左邻居的边）：沿正上/正下搜索端点，端点上与正上/正下邻居混合 ----
    if (e0.r > 0.5)
    {
        int up = 1;
        int dn = 1;
        for (int i = 1; i <= uMaxSteps; ++i)
        {
            if (Edges(TexCoords + vec2(0.0, uTexel.y * float(i))).r > 0.5) { ++up; } else { break; }
        }
        for (int i = 1; i <= uMaxSteps; ++i)
        {
            if (Edges(TexCoords - vec2(0.0, uTexel.y * float(i))).r > 0.5) { ++dn; } else { break; }
        }
        float L   = float(up + dn - 1);
        bool  atUp = (up == 1);
        bool  atDn = (dn == 1);
        if (atUp && atDn) { wUp += 0.25; wDn += 0.25; }
        else
        {
            if (atUp) { wUp += 0.5 / L; }
            if (atDn) { wDn += 0.5 / L; }
        }
    }

    //  ---- 横边（与下邻居的边）：沿正左/正右搜索端点，端点上与正左/正右邻居混合 ----
    if (e0.g > 0.5)
    {
        int lf = 1;
        int rt = 1;
        for (int i = 1; i <= uMaxSteps; ++i)
        {
            if (Edges(TexCoords - vec2(uTexel.x * float(i), 0.0)).g > 0.5) { ++lf; } else { break; }
        }
        for (int i = 1; i <= uMaxSteps; ++i)
        {
            if (Edges(TexCoords + vec2(uTexel.x * float(i), 0.0)).g > 0.5) { ++rt; } else { break; }
        }
        float L   = float(lf + rt - 1);
        bool  atLf = (lf == 1);
        bool  atRt = (rt == 1);
        if (atLf && atRt) { wLf += 0.25; wRt += 0.25; }
        else
        {
            if (atLf) { wLf += 0.5 / L; }
            if (atRt) { wRt += 0.5 / L; }
        }
    }

    //  权值 = "这个像素里该有多少是邻居的颜色"。总权超过 1 就等比缩回去（拐角处会同时拿到
    //  两个方向的权），避免把像素混成纯邻居色。
    float total = wUp + wDn + wLf + wRt;
    if (total > 1.0)
    {
        float k = 1.0 / total;
        wUp *= k; wDn *= k; wLf *= k; wRt *= k;
    }

    vec3 outc = c
        + wUp * (texture(uColor, TexCoords + vec2(0.0, uTexel.y)).rgb - c)
        + wDn * (texture(uColor, TexCoords - vec2(0.0, uTexel.y)).rgb - c)
        + wLf * (texture(uColor, TexCoords - vec2(uTexel.x, 0.0)).rgb - c)
        + wRt * (texture(uColor, TexCoords + vec2(uTexel.x, 0.0)).rgb - c);

    FragColor = vec4(outc, 1.0);
}