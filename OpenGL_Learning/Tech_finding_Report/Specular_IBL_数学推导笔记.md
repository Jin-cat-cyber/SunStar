# Specular IBL 数学推导笔记

> 基于 LearnOpenGL《Specular IBL》一文,把其中所有数学推导展开、补齐原文跳过的步骤。
> 目标:搞清楚"间接镜面反射"这一项为什么能离线预计算、每一步公式从哪来、代码对应哪一行。
>
> 用 Markdown Preview Enhanced 插件预览,可渲染 LaTeX 公式。

---

## 0. 记号约定

| 记号 | 含义 |
| --- | --- |
| $n$ | 表面法线 |
| $\omega_i$ | 入射光方向 |
| $\omega_o$ | 出射(视线)方向,即 $V$ |
| $h$ | 半向量 $\frac{\omega_i+\omega_o}{\|\omega_i+\omega_o\|}$ |
| $\Omega$ | 法线 $n$ 正方向的半球 |
| $N\cdot L$ | $n\cdot\omega_i=\cos\theta_i$ |
| $N\cdot V$ | $n\cdot\omega_o=\cos\theta_o$ |
| $V\cdot H$ | $\omega_o\cdot h$ |
| $r$ | 感知粗糙度(perceptual roughness),$0\sim 1$ |
| $\alpha$ | NDF 中使用的粗糙度平方,$\alpha = r^2$ |
| $L_i$ | 入射辐射度(radiance) |
| $F_0$ | 法线入射($0^\circ$)时的菲涅耳反射率,材质基础反射色 |

---

## 1. 反射方程与镜面项

PBR 的完整反射方程:

$$
L_o(p,\omega_o)=\int_\Omega\left(k_d\frac{c}{\pi}+k_s\frac{D F G}{4(\omega_o\cdot n)(\omega_i\cdot n)}\right)L_i(p,\omega_i)\,(n\cdot\omega_i)\,d\omega_i
$$

其中 $D F G / (4(\omega_o\cdot n)(\omega_i\cdot n))$ 就是 Cook-Torrance 镜面 BRDF:

$$
f_r(p,\omega_i,\omega_o)=\frac{D F G}{4(\omega_o\cdot n)(\omega_i\cdot n)}
$$

- $D$ = 法线分布函数(NDF)
- $F$ = 菲涅耳(Fresnel)
- $G$ = 几何遮蔽-阴影函数(Geometry)

### 为什么漫反射能预计算,镜面不能

漫反射项 $k_d\,c/\pi$ 是**常数**,可提到积分外:

$$
L_o^{\text{diff}} = \left(k_d\frac{c}{\pi}\right)\int_\Omega L_i\,(n\cdot\omega_i)\,d\omega_i
$$

积分只依赖法线 $n$,一张以 $n$ 为索引的立方体贴图(辐照度图)就够查了。

镜面项 $f_r$ 同时依赖 $\omega_i$ **和** $\omega_o$。要对所有 $\omega_i\times\omega_o$ 组合预计算,组合数爆炸,实时/离线都不可行。

---

## 2. Split-Sum 近似

Epic Games 的方案:把镜面积分硬拆成两个独立积分的乘积。

$$
L_o(p,\omega_o)\approx\left(\int_\Omega L_i(p,\omega_i)\,d\omega_i\right)\left(\int_\Omega f_r(p,\omega_i,\omega_o)\,(n\cdot\omega_i)\,d\omega_i\right)
$$

> **注意**:严格说 $\int(f\cdot g)\neq\left(\int f\right)\left(\int g\right)$,所以这是**有偏近似**。

两部分各有职责:

1. **预滤波环境贴图**(pre-filtered environment map)—— 只依赖光照 $L_i$,按粗糙度预卷积成 mip 链;
2. **BRDF 积分贴图**(BRDF LUT)—— 只依赖 BRDF($n\cdot\omega_o$、粗糙度、$F_0$),预积分成一张 2D 纹理。

### 它牺牲了什么

第一部分卷积时不知道视角方向,Epic 假设:

$$
V = R = N
$$

即"视角方向 = 反射方向 = 法线"。后果是**掠射角(grazing angle)下的镜面反射不准确**——恰恰是菲涅耳最强、最该亮的地方。日常视觉可接受,所以几乎人人采用。

---

## 3. 蒙特卡洛积分

不求所有(理论上无限个)采样值,而是随机采 $N$ 个样本取平均:

$$
\int_a^b f(x)\,dx \approx \frac{1}{N}\sum_{i=0}^{N-1}\frac{f(x_i)}{p(x_i)}
$$

- $p(x_i)$ = 概率密度函数(pdf),样本 $x_i$ 被采到的概率密度;
- 均匀采样 $\Rightarrow$ 无偏,$N\to\infty$ 精确收敛;
- 采样偏向某些区域 $\Rightarrow$ **有偏**,但**收敛更快**(方差更小)。图形学里"视觉正确"即可,普遍接受有偏。

**拟蒙特卡洛(QMC)**:用低差异序列取代纯随机,样本分布更均匀,收敛更快。

---

## 4. 低差异序列

### 4.1 Van der Corput 序列(基数 2)

把整数 $i$ 的二进制表示**按位反转**放到小数点后:

| $i$ | 二进制 | 反转 | 十进制 |
| --- | --- | --- | --- |
| 1 | `1` | `0.1` | 0.5 |
| 2 | `10` | `0.01` | 0.25 |
| 3 | `11` | `0.11` | 0.75 |
| 4 | `100` | `0.001` | 0.125 |

序列 $0.5,0.25,0.75,0.125,0.625,\dots$ 每个新点都落在当前最大空隙里,这就是"低差异"。

GLSL 里用五行位运算完成 32 位反转,最后乘 $2^{-32}$ 归一化到 $[0,1)$:

```c++
float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);              // 交换高/低 16 位
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u); // 相邻 1 位
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u); // 相邻 2 位
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u); // 相邻 4 位
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u); // 相邻 8 位
    return float(bits) * 2.3283064365386963e-10;      // / 0x100000000
}
```

### 4.2 Hammersley 序列

二维低差异序列,第一维均匀等分,第二维用 Van der Corput:

$$
\text{Hammersley}(i,N)=\left(\frac{i}{N},\ \Phi_2(i)\right)
$$

即采样函数拿到的两个随机数 $\xi_1=Xi.x,\ \xi_2=Xi.y$。

---

## 5. GGX / Trowbridge-Reitz 法线分布

$$
D(h)=\frac{\alpha^2}{\pi\left((n\cdot h)^2(\alpha^2-1)+1\right)^2},\qquad \alpha=r^2
$$

记 $\cos\theta=n\cdot h$,则

$$
D(h)=\frac{\alpha^2}{\pi\left(\cos^2\theta\,(\alpha^2-1)+1\right)^2}
$$

GGX 满足归一化条件:

$$
\int_\Omega D(h)\,(n\cdot h)\,d\omega = 1
$$

这是后面选采样密度为合法 pdf 的依据。

---

## 6. 重要性采样(GGX)

### 6.1 采样密度选择

镜面能量集中在镜面波瓣内,波瓣形状由 NDF 决定。让样本密度正比于 NDF:

$$
p(h)=D(h)\,(n\cdot h)
$$

由第 5 节归一化条件,$p(h)$ 在全半球积分为 1,是合法概率密度。

### 6.2 方位角 $\phi$ 均匀采样

因为 NDF 各向同性(不含 $\phi$),波瓣绕法线旋转对称。联合密度为

$$
p(\theta,\phi)=D(\theta)\cos\theta\,\sin\theta
$$

把 $\theta$ 积掉,$\phi$ 的边缘分布是常数 $\frac{1}{2\pi}$,即均匀分布。逆变换:

$$
\phi = 2\pi\,\xi_1
$$

代码 `float phi = 2.0 * PI * Xi.x;`,不用任何推导。

> 各向异性材质(拉丝金属、头发)的 NDF 依赖 $\phi$,此处不再成立,需 $\alpha_x,\alpha_y$ 两套粗糙度。

### 6.3 极角 $\theta$ 逆变换采样

**第 1 步**——$\theta$ 的边缘密度:

$$
p(\theta)=2\pi\,D(\cos\theta)\cos\theta\sin\theta=\frac{2\alpha^2\cos\theta\sin\theta}{\left(\cos^2\theta(\alpha^2-1)+1\right)^2}
$$

**第 2 步**——累积分布:

$$
P(\theta)=\int_0^\theta p(t)\,dt=\int 2\alpha^2\,\frac{\cos t\sin t}{\left(\cos^2 t(\alpha^2-1)+1\right)^2}\,dt
$$

换元 $u=\cos^2 t$,$du=-2\cos t\sin t\,dt$:

$$
P=-\alpha^2\int\frac{du}{\left(u(\alpha^2-1)+1\right)^2}
$$

套公式 $\int\frac{du}{(mu+c)^2}=-\frac{1}{m(mu+c)}$($m=\alpha^2-1,c=1$):

$$
P=\frac{\alpha^2}{(\alpha^2-1)\left(u(\alpha^2-1)+1\right)}\Bigg|_{u=1}^{u=\cos^2\theta}
=\frac{\alpha^2}{(\alpha^2-1)\left(\cos^2\theta(\alpha^2-1)+1\right)}-\frac{1}{\alpha^2-1}
$$

**第 3 步**——验端点:$\theta=0$ 得 $P=0$;$\theta=\frac{\pi}{2}$ 得 $P=1$,是合法 CDF。

**第 4 步**——逆变换,令 $P=\xi_2$,反解 $\cos\theta$:

$$
\xi_2=\frac{\alpha^2}{(\alpha^2-1)\left(\cos^2\theta(\alpha^2-1)+1\right)}-\frac{1}{\alpha^2-1}
$$

$$
\Rightarrow\quad \cos^2\theta=\frac{1-\xi_2}{1+(\alpha^2-1)\xi_2}
$$

$$
\Rightarrow\quad \cos\theta=\sqrt{\frac{1-\xi_2}{1+(\alpha^2-1)\xi_2}}
$$

代码一行对应:

```c++
float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
```

### 6.4 半向量 → 入射方向

采出的是半向量 $h$,入射方向 $L$ 由反射定律反推:

$$
L = 2(\omega_o\cdot h)\,h - \omega_o
$$

---

## 7. 预滤波卷积

卷积着色器对每个样本累加环境光照,用 $N\cdot L$ 加权:

$$
\text{prefilteredColor} = \frac{\sum_i L_i \cdot (n\cdot L_i)}{\sum_i (n\cdot L_i)}
$$

$N\cdot L=\cos\theta_i$ 是立体角投影因子;贡献小的样本权重也小。最后除以权重和做归一化。

粗糙度 $\rightarrow$ mip 级别映射:

$$
r=\frac{\text{mip}}{\text{maxMipLevels}-1}
$$

mip 越高,卷积采样向量越分散,反射越模糊。

---

## 8. 伪像修复

### 8.1 立方体贴图接缝

粗 mip 分辨率低,OpenGL 默认不在六个面之间插值,产生接缝。修复:

```c++
glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
```

### 8.2 亮点 / 点状图案

高粗糙度 mip 下采样不足,高频 HDR 细节在亮斑周围产生点。修复:直接采样源环境贴图更模糊的 mip,省掉昂贵重卷积。

样本的实际 pdf(从 $h$ 采样转到 $L$ 采样,含雅可比):

$$
p = \frac{D(n\cdot h)}{4(\omega_o\cdot h)}
$$

单个纹素占的立体角(球面总 $4\pi$,纹素总数 $6\,res^2$):

$$
sa_{\text{texel}} = \frac{4\pi}{6\cdot res^2}
$$

单个样本覆盖的立体角:

$$
sa_{\text{sample}} = \frac{1}{N\cdot p}
$$

换算成 mip 级别(面积每级 ÷4,$\frac12\log_2$ 即 $\log_4$):

$$
\text{mipLevel}=\frac12\log_2\frac{sa_{\text{sample}}}{sa_{\text{texel}}}
$$

---

## 9. BRDF LUT

### 9.1 把 $F_0$ 提出积分

Schlick 近似:

$$
F=F_0+(1-F_0)(1-\omega_o\cdot h)^5
$$

令 $m=(1-\omega_o\cdot h)^5$:

$$
F=F_0+(1-F_0)m=F_0(1-m)+m
$$

把 BRDF 里的菲涅耳项单独提出:

$$
\int_\Omega f_r(n\cdot\omega_i)\,d\omega_i
=\int_\Omega \frac{f_r}{F}\,F\,(n\cdot\omega_i)\,d\omega_i
$$

代入 $F$ 并拆成两项,$F_0$ 是常数可提出:

$$
= F_0\underbrace{\int_\Omega\frac{f_r}{F}(1-m)(n\cdot\omega_i)\,d\omega_i}_{A}
\ +\ \underbrace{\int_\Omega\frac{f_r}{F}\,m\,(n\cdot\omega_i)\,d\omega_i}_{B}
$$

结果 $=F_0 A+B$:

- $A$ = **scale**,$F_0$ 的系数,存 LUT 的 R 通道;
- $B$ = **bias**,与 $F_0$ 无关的偏移,存 G 通道。

横轴 $N\cdot V$,纵轴粗糙度。因为 A、B 都只依赖 $n\cdot\omega_o$ 和粗糙度,与 $F_0$、与具体环境贴图无关。

### 9.2 $G_{vis}$ 的由来(被积函数 ÷ pdf)

去掉菲涅耳后的 BRDF:

$$
\frac{f_r}{F}=\frac{D G}{4(n\cdot\omega_i)(n\cdot\omega_o)}
=\frac{D G}{4(N\cdot L)(N\cdot V)}
$$

被积函数(先不提 $F$):

$$
\frac{f_r}{F}(n\cdot\omega_i)=\frac{D G}{4(N\cdot V)}
$$

采样 pdf(含 6.4 的雅可比 $1/(4V\cdot H)$):

$$
p(L)=\frac{D(n\cdot h)}{4(\omega_o\cdot h)}=\frac{D(N\cdot H)}{4(V\cdot H)}
$$

蒙特卡洛单个样本贡献 = 被积函数 ÷ pdf:

$$
\frac{\dfrac{D G}{4(N\cdot V)}}{\dfrac{D(N\cdot H)}{4(V\cdot H)}}
=\frac{G(V\cdot H)}{(N\cdot H)(N\cdot V)}
$$

这就是代码里的 $G_{vis}$:

```c++
float G_Vis = (G * VdotH) / (NdotH * NdotV);
```

> 第 8.2 节的 pdf 与这里的 pdf 是同一个 $D(N\cdot H)/(4(V\cdot H))$——都在做"半向量采样 → 入射方向采样"的换算。雅可比 $1/(4(V\cdot H))$ 是反射采样标志性系数。

### 9.3 几何项的 $k$:IBL 与直接光不同

$$
k_{\text{direct}}=\frac{(\alpha+1)^2}{8},\qquad k_{\text{IBL}}=\frac{\alpha^2}{2}
$$

BRDF 卷积是 IBL 一部分,用 $k_{\text{IBL}}$。代码有个坑:

```c++
float a = roughness;          // 直接赋 roughness,没平方!
float k = (a*a) / 2.0;        // = roughness^2 / 2
```

此处 $a$ 不是 $\alpha=r^2$ 而是 $r$ 本身,相当于"假装平方过",这样结果才与 Epic 官方 LUT 一致。

物理直觉:IBL 的 $k$ 比直接光小(遮蔽更弱),因为 IBL 视作无限远均匀环境,无直接光那种尖锐遮蔽情形。

---

## 10. 最终缝合

$$
F = \text{FresnelSchlickRoughness}(N\cdot V, F_0, r)
$$

$$
k_S=F,\qquad k_D=(1-F)\,(1-\text{metallic})
$$

$$
L_o = \underbrace{\text{irradiance}(n)\cdot \text{albedo}}_{\text{diffuse}}
\ +\ \underbrace{\text{prefiltered}(R, r\cdot\text{maxLod})\cdot\left(F\cdot A+B\right)}_{\text{specular}}
$$

$$
\text{ambient}=\left(k_D\cdot\text{diffuse}+\text{specular}\right)\cdot ao
$$

其中 $R=\text{reflect}(-\omega_o,n)$。**specular 不再乘 $k_S$**,因为里面的 $F$ 已包含菲涅耳系数(即 $F\cdot A$)。

---

## 附录:完整采样伪代码

```c++
const uint SAMPLE_COUNT = 1024u;
float totalWeight = 0.0;
vec3 prefilteredColor = vec3(0.0);

for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
    vec2 Xi = Hammersley(i, SAMPLE_COUNT);          // 低差异序列
    vec3 H  = ImportanceSampleGGX(Xi, N, roughness); // 采半向量
    vec3 L  = normalize(2.0*dot(V,H)*H - V);         // 反射得入射方向

    float NdotL = max(dot(N,L), 0.0);
    if (NdotL > 0.0) {
        prefilteredColor += texture(environmentMap, L).rgb * NdotL;
        totalWeight      += NdotL;
    }
}
prefilteredColor /= totalWeight;
```

---

## 进阶阅读

- [Real Shading in Unreal Engine 4 (Karis 2013)](http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf) —— split-sum 出处
- [Moving Frostbite to PBR (Lagarde & de Rousiers)](https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf)
- [Image Based Lighting (Chetan Jags)](https://chetanjags.wordpress.com/2015/08/26/image-based-lighting/)
