# 小行星 LOD · 尝试与废弃技术报告

> 版本：SpaceShip18.0.cpp + `Globals.h` + `Procedural.h/.cpp` + `res/shader/00_SpaceShip/RockPoint/`
> 定稿日期：2026-09-07
> 结论：**LOD 废弃为默认路径，代码以 `lodEnabled = false` 开关保留在 18.0，主线回退到 17.A（A+C 视锥剔除）**
> 配套文档：[[小行星视锥剔除_方案C阴影渲染分离技术报告]](小行星视锥剔除_方案C阴影渲染分离技术报告.md)

---

## 一、目标与动机

**原始诉求**：远处的小行星在相机移动时出现**闪点噪点**（亚像素三角面光栅化的覆盖率抖动）。

**设想**：用 LOD 把远距离小行星从"192 三角的全模"换成"**点精灵**"——
- 1 个图元代替 192 三角 → 省几何；
- 亚像素下从"硬边三角抖动"变成"平滑微粒" → 期望同时消掉闪点。

**定位**：属于"为未来更多物体/3D 噪声预留余量"的储备性改造（当时性能并不吃紧）。

---

## 二、实现（方案一：近全模 / 远点精灵）

### 2.1 数据与判据

`Globals.h` 新增 LOD 状态：

```cpp
inline std::vector<glm::mat4> gNear;          // LOD0 近桶：全模矩阵
inline std::vector<glm::vec4> gFarPos;        // LOD1 远桶：xyz=位置, w=点径(像素)
inline unsigned int rockNearCount = 0;
inline unsigned int rockFarCount  = 0;
inline std::vector<unsigned char> gLodLevel;  // 每实例当前档(0近/1远)，迟滞记忆
inline unsigned int rockPointVAO = 0;
inline unsigned int rockPointVBO = 0;
inline bool lodEnabled = false;               // 总开关（废弃后默认关闭）
```

**屏幕占比**判据（`Procedural.cpp`）：

$$
\text{screenPx} \;=\; \frac{\text{worldRadius}}{\text{dist}}\cdot\frac{\text{viewportH}}{2\tan(\text{FOV}/2)}
$$

**分桶 + 迟滞**（`RockViewFrustumCull`）：

```cpp
float px = ScreenSizePx(r, dist, vpH, fovRad);
unsigned char lv = gLodLevel[i];
if (px < 3.0f)      lv = 1;     // 降级 → 点
else if (px > 6.0f) lv = 0;     // 升级 → 全模
gLodLevel[i] = lv;              // 3~6px 之间维持上一帧
if (SphereInFrustum(fp, c, r)) {
    if (lv == 1) gFarPos.push_back(glm::vec4(c, glm::clamp(px * 1.5f, 2.0f, 6.0f)));
    else         gNear.push_back(gRockMatrices[i]);
}
```

### 2.2 绘制

- **G-Buffer**：近桶 → instanced 全模 mesh；远桶 → `GL_POINTS` 点精灵（也写进同一个 gBuffer，由延迟光照统一着色）；
- **Shadow Pass**：**刻意不做 LOD**，仍用 C 判据的全集 + 全模（保星环/火星上的带子阴影）。

---

## 三、结果与问题

### 3.1 闪烁黑点（两个坑叠加）

**现象**：相机移动时出现不断闪烁的**黑点**。

**坑 A：在不透明 G-Buffer 里用 `albedo * a` 做"软边" → 边缘变黑**

```glsl
float a = smoothstep(0.5, 0.22, d);
gAlbedo = vec4(albedo * a, 0.0);      // ← 本意是软边，实际把边缘 albedo 乘成 0 = 黑圈
```

G-Buffer 是**不透明**的、**没有 alpha 混合**：`a → 0` 不是"融进背景"，而是**颜色变黑**。
每颗粒子变成"亮心 + 黑圈"，在 2~6px 尺寸下黑圈占大半 → 黑点。

**坑 B：`out vec3 gAlbedo` 丢掉 `.a` → metallic 变 1 → 粒子变黑金属**

- G-Buffer 的 2 号附件是 **RGBA**，其中 `.a` 就是 **metallic**（延迟光照 `texture(gAlbedo,...).a`）；
- 原版小行星 shader 是 `out vec4 gAlbedo; gAlbedo = vec4(albedo, 0.0);`；
- 点精灵 shader 误写 `out vec3 gAlbedo` → 赋值 `vec4(...)` 被截断、`.a` 未定义（驱动通常给 1.0）；
- 于是 `metallic = 1` → **kD 被乘 0、漫反射项消失**，只剩粗糙度 0.9 的镜面 IBL（太空环境图很暗）→ **粒子基本全黑**。

**修复**：

```glsl
layout (location = 2) out vec4 gAlbedo;      // 必须 vec4
...
if (dot(c, c) > 0.25) discard;               // 用 discard 裁圆盘(硬边)，不再乘 a
gAlbedo = vec4(albedo, 0.0);
```

修复后黑点消失。

> **教训**：
> 1. **软边必须靠混合实现**——写进不透明 G-Buffer 的"alpha"只等于颜色，不等于透明度（与"加法 pass 不会遮挡""大气层必须写深度"同类）。
> 2. **G-Buffer 有通道契约**：albedo 必须是 vec4 且 `.a` = metallic，写错类型会静默污染光照。

### 3.2 LOD 切换 pop（无法靠调参消除）

**现象**：画面中固定距离处，一颗小点"啪"地变成有细节的石头。

**分析**：尺寸其实是**连续**的——
- 远档点径 = `clamp(px*1.5, 2, 6)`，上限 **6px**；
- 升级阈值 = `px > 6`；
- 切换瞬间：点约 6px、石头约 6px，**大小接得上**。

真正跳的是**外观**：远档是**匀色圆盘**（单色 albedo + 延迟光照），近档是**带纹理、有法线起伏的真石头**。这是"点 ↔ mesh"方案的**固有代价**，调阈值无法消除。

**一个被纠正的错误判断**：曾以为"把升级阈值调大（8~10px）能软化 pop"，这是**错的**——因为点的尺寸上限被 clamp 在 6px，阈值调到 10px 会变成"6px 点 → 10px 石头"，**突变更大**。

**可用的软化手段（未采用）**：
1. **每实例阈值错开**（伪随机偏移 ±1.5px）→ pop 从"成圈集体跳"变"错峰零散跳"，最有效；
2. 迟滞带加宽（如 2.5 / 9px）→ 减少来回翻转；
3. 让远档外观更接近石头（按太阳做粗糙 Lambert）→ 收效有限。

### 3.3 性能收益低（ROI 不划算）

| Pass | 量级 | LOD 后 | 被优化？ |
|---|---|---|---|
| **Shadow Pass** | `gShadowVisible × 192 × 6 面`（几何着色器每三角复制 6 面） | **完全不变** | ❌ |
| G-Buffer | `~19k × 192` | 近桶少量×192 + 远桶点精灵 | ✅ |

**关键**：为了让小行星带的阴影**保留在火星/尘埃星环上**，Shadow Pass **刻意未做 LOD**；而它恰是**最大的一块开销**。LOD 只削减了较小的 G-Buffer 部分 → **净收益小**，却付出了 **pop + 远场变"碎点"的画面代价**。

**结论：投入（视觉损失）> 产出（帧率）。**

---

## 四、决策

1. **LOD 废弃为默认路径**；
2. **代码保留**：`18.0` 以 `lodEnabled = false` 开关封存（关闭时 `gFarPos` 恒空、远桶 draw 自动跳过，画面等同 17.A）；
3. **主线回退到 17.A**：只保留 **A（相机视锥剔除）+ C（阴影独立判据）** 这套已验证的收益；
4. 点 VAO/VBO、点 shader、初始化代码**保留不删**（关闭时零开销），作为未来储备。

---

## 五、经验沉淀

1. **LOD 的判据（相机距离 / 屏幕占比）只适用于"渲染细节"，不适用于"阴影投射"**。阴影看的是光几何（见 C 方案的能否投影判据），远小行星同样可能投影到可见的星环上。
2. **软边 ≠ 透明度**（不透明 G-Buffer 里只会变黑）；需要软边就得用**独立的混合 pass**，而那样又会脱离延迟光照一致性——**亚像素尘埃不值得**。
3. **G-Buffer 是带契约的**：`albedo.a=metallic`、MRT 类型必须与着色器输出一致，否则静默出错。
4. **LOD 只在"几何量真正占主导"时才划算**。本例中占主导的是阴影 Pass 的 6× 重复，LOD 碰不到它，因此收益有限。
5. **亚像素闪烁是采样/抗锯齿问题，LOD 治不了**。正解是抗锯齿（延迟管线 + MSAA 是经典难题）或压缩远场可见量，属于另一条技术路线。

---

## 六、未来启用条件与前置工作

**何时值得重新启用**：
- 场景物体数量/密度大幅上升（几何量成为主导瓶颈）；
- 相机频繁贴近物体（近/远细节差异显著）。

**启用时必须补的前置**：
1. **每实例阈值错开**（软化 pop）；
2. 迟滞带加宽；
3. 若要软边 → 走**独立混合 pass**，明确接受"与延迟光照略不一致"；
4. 若同时要削阴影 → 必须用**低模网格**（如顶点聚类生成的 `rock_low`）而非点精灵，且**仍进阴影贴图**（否则丢失星环上的带子阴影）。

---

## 七、相关

- 视锥剔除（A+C）与阴影判据：[[小行星视锥剔除_方案C阴影渲染分离技术报告]](小行星视锥剔除_方案C阴影渲染分离技术报告.md)
- 全屏 Pass 遮挡修复（不透明/深度类教训）：[[大气散射全屏Pass遮挡修复_深度测试与深度输出技术报告]](大气散射全屏Pass遮挡修复_深度测试与深度输出技术报告.md)
- 尘埃星环（受小行星带阴影影响的体积效果）：[[尘埃星环体积渲染参数调节说明书]](尘埃星环体积渲染参数调节说明书.md)
