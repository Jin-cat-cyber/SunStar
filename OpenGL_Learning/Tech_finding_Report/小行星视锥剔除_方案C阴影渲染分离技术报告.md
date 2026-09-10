# 小行星视锥剔除 · 方案C阴影与渲染分离技术报告

> 版本：SpaceShip17.0 / SpaceShip17.A.cpp + `Globals.h` + `Procedural.h/.cpp`
> 定稿日期：2026-09-07
> 配套文档：
> - [[行星大气散射技术报告]](行星大气散射技术报告.md)、[[尘埃星环体积渲染参数调节说明书]](尘埃星环体积渲染参数调节说明书.md)

---

## 一、背景与目标

场景含 **5 万颗实例化小行星**（`rock.obj`，98 顶点/192 三角），在**两个 Pass** 里被绘制：

1. **G-Buffer（延迟几何）**：每帧 50000 × 192 ≈ 9.6M 三角；
2. **Shadow Pass（点光 cubemap）**：几何着色器每三角复制 6 面 → 50000 × 192 × 6 ≈ **57.6M** 次三角发射——这是最大的性能消耗点。

目标：在**保留小行星带投射到火星/星环上的阴影**的前提下，削掉这部分顶点/几何开销。核心手段是**视锥剔除**——只画"当前镜头用得着"的实例。

---

## 二、方案 A：G-Buffer 与 Shadow Pass 共用同一套相机视锥（第一版）

### 实现
- CPU 侧预计算每颗小行星的**包围球**（圆心=矩阵平移、半径=网格固有半径×缩放）；
- 每帧从相机 `projection*view` 提取 6 个视锥平面，做**球-视锥**相交测试，得到 `gVisible`；
- G-Buffer 与 Shadow Pass **都**用 `gVisible` + `rockVisibleCount` 绘制。

### 效果
- 实测 `visibleCount≈18947/50000`（约 38% 在视锥内），**三角形处理量约砍 62%**，画面无变化（视野外本来看不到）——性能收益直接到位。

### 问题（方案 A 的固有缺陷）
**G-Buffer 关心的是"渲染可见性"，Shadow Pass 关心的是"阴影投射能力"，两者不是同一件事。**

1. **视野外但会投影的小行星被误剔**：相机视锥外的某颗小行星，其影子可能落在**可见的火星/星环表面**上。A 用相机视锥剔它们 → 这些影子**丢失**。
2. **"移动时边缘阴影突然消失"**：相机的近平面随移动快速扫过小行星带，边界附近的小行星瞬间"进/出视锥" → 阴影在边缘**跳变/闪没**——这是"用渲染可见性当阴影投射判据"带来的不稳定。
3. **帧错位（相机状态滞后一帧）**：剔除块在 follow 模式相机更新**之前**执行，用的是上一帧相机 → cull 与渲染有 1 帧错位。

---

## 三、方案 C：阴影 Pass 用独立判据（光锥射线判据）

### 核心思路
"能否投影到可见面" ≠ "是否在相机视锥内"。正确的阴影投射判据是：

> **一颗小行星 p 能投影到相机看得见的表面 ⇔ 从光源穿过 p 的射线，在这个位置之后进入/穿过相机视锥。**

即：p 的影子要落在某个可见表面 S 上，S 必须**在 p 后方（远离光源一侧）且能被相机看到**。

### 数学（CanCastVisibleShadow）
光源 $\mathbf L$，小行星 $\mathbf p$，单位方向 $\mathbf r=\frac{\mathbf p-\mathbf L}{\lVert \mathbf p-\mathbf L\rVert}$，光源到 p 的距离 $L_p=\lVert\mathbf p-\mathbf L\rVert$。光线 $\mathbf x(t)=\mathbf L+t\,\mathbf r$ 在视锥内的条件为所有平面 $n_i\cdot\mathbf x+w_i\ge0$ 同时成立，解得区间：

$$
t_{enter}=\max_{n_i\cdot r>0}\frac{-w_i-n_i\cdot\mathbf L}{n_i\cdot\mathbf r},\qquad
t_{exit}=\min_{n_i\cdot r<0}\frac{-w_i-n_i\cdot\mathbf L}{n_i\cdot\mathbf r}
$$

**保留判据**：

$$
\text{保留} \iff t_{enter}\le t_{exit}\ \ \text{且}\ \ L_p < t_{exit}
$$

（射线穿过视锥，且 p 未越过视锥远端——即 p 的影子能落到某个可见面上。）

### 两套集合、两个绘制点
- `gVisible`：相机视锥（G-Buffer 用，= 渲染可见）；
- `gShadowVisible`：光锥判据（Shadow Pass 用，= 能投影到可见面）；
- 理论上 `gShadowVisible ⊇ gVisible`（"能投影"天然包含"能看见"），是正确超集。

---

## 四、迭代中踩到并修掉的小问题（重点）

### 问题 1：共享实例 VBO，两个 Pass 必须各自上传自己的子集
- **症状**：G-Buffer 正常、Shadow Pass 阴影残缺/错误。
- **根因**：G-Buffer 与 Shadow 复用**同一个实例 VBO**（`rockInstanceVBO`），但当时只有 cull 里上传了 `gVisible`；Shadow 直接画 `rockShadowVisibleCount` 个，**读的却是 VBO 里的 `gVisible` + 越界垃圾**（初始全量上传已注释，VBO 后段为空）。
- **修复**：每个 Pass 绘制**前**各自 `glBufferSubData` 上传自己那批矩阵：
  - Shadow：上传 `gShadowVisible`，画 `rockShadowVisibleCount`；
  - G-Buffer：上传 `gVisible`，画 `rockVisibleCount`。

### 问题 2：`CanCastVisibleShadow` 判据方向错（`tEnter>Lp` → `Lp<tExit`）
- **症状**：**视锥体内无阴影**，**屏幕边缘/夹角处却有阴影并呈锥形**，且**相机移动时边缘阴影直接消失**。
- **根因**：初版写成 `return tEnter > Lp;`，即**只保留"光源与视锥之间、尚未进视锥"的小行星**。

  沿光线的三种位置：

  | p 的位置 | 区间 | 能否投影 | 旧条件 `tEnter>Lp` | 新条件 `Lp<tExit` |
  |---|---|---|---|---|
  | 视锥前 | $L_p<t_{enter}$ | ✅ | ✅ 保留 | ✅ 保留 |
  | **视锥内** | $t_{enter}\le L_p\le t_{exit}$ | ✅ | ❌ **剔除** | ✅ 保留 |
  | 视锥后 | $L_p>t_{exit}$ | ❌ | ❌ 剔除 | ❌ 剔除 |

  **旧条件把"视锥内"（= 相机能看见、最该互相投影的那批）整段剔没了**，只有那层"视锥前的薄壳"进阴影贴图 → 中心无影、边缘锥形、且薄壳以近平面为界对相机朝向极敏感 → **移动时边缘阴影跳变闪没**。

- **修复**：`return Lp < tExit;`——保留"未越过视锥远端"的整个光锥（含视锥内）。集合随相机**平缓连续变化**，阴影稳定。
- **几何直觉（俯视截面）**：
  - 旧条件只在**视锥前薄壳**里选投影源 → 中心无影、边缘锥形；
  - 新条件选穿越视锥的**实心光锥** → 视锥内小行星互投影，正常。

### 问题 3：follow 模式相机 1 帧错位
- **症状**：剔除边界与画面有 1 帧错位（仅 follow 模式）。
- **根因**：follow 相机更新块在 `RockViewFrustumCull` **之后**执行。
- **修复**：把 `if (currentMode == MODE_FOLLOW){...}` 块**挪到 cull 之前**（`ship.Update` 之后）。free/remote 模式帧内相机不变，本就无此问题（鼠标输入在 `glfwPollEvents`，属正常输入延迟，不要动）。

### 问题 4：附属优化
- **实例缓冲用 `GL_DYNAMIC_DRAW`**（每帧重填可见子集），去掉冗余的初始全量上传；
- `RocksModelMatricesInit` 结束后 `delete[] modelMatrices;`（数据已入 `gRockMatrices` 与 VBO）；
- **视锥平面归一化**：只归一化**法线（3D）**部分（`n*invLen, w*invLen`），**不要用 `glm::normalize(vec4)`**（会除以四维模长，导致球-视锥测试的 `+r` 项量纲错）。

---

## 五、自测校验

`RockViewFrustumCull` 内临时打印（前几帧）：

```
[cull] center-in=1  behind-out=1  visibleCount=18947 / 50000
```

| 现象 | 诊断 |
|---|---|
| `center-in=0` 或 `behind-out=0` | 平面符号/内部判别反了 |
| `visibleCount≈50000` | 剔除恒真（没生效） |
| `visibleCount≈0` | 全剔（符号反） |
| `rockShadowVisibleCount < visibleCount` | `CanCastVisibleShadow` 判据方向反了（应为超集） |

---

## 六、结论与经验

1. **渲染可见性 ≠ 阴影投射能力**。G-Buffer 用"相机视锥"，Shadow/cubemap 用"能否投影到可见面"，**两套判据，两套集合**。这是方案 C 的核心。
2. **阴影贴图是一张"被所有遮挡物填满"的共享深度图**——任何要投影的对象都必须在该集合里，否则其影子消失；而全屏体积类（星环）只做"采样者"，不需进深度图本身。
3. **共享实例缓冲，各 Pass 绘制前必须上传自己的子集**，否则会读到别的集合的矩阵。
4. **"视锥内"是最该互投影的对象**，任何把这一步剔掉的判据（如只留视锥前）都会造成"中心无影 + 边缘锥形 + 移动闪没"。
5. CPU 静态实例 + 每帧剔除（每帧 5 万次球测 + 一次子集上传，CPU 远低于 1ms）是小场景最轻量有效的方案。

---

## 七、相关

- 全屏 Pass 遮挡修复（深度测试与深度输出）：[[大气散射全屏Pass遮挡修复_深度测试与深度输出技术报告]](大气散射全屏Pass遮挡修复_深度测试与深度输出技术报告.md)
- 大气散射（同套 ray-march + 解析相交范式）：[[行星大气散射技术报告]](行星大气散射技术报告.md)
- 尘埃星环（点光 cubemap 阴影采样、raySphere 遮挡）：[[尘埃星环体积渲染参数调节说明书]](尘埃星环体积渲染参数调节说明书.md)
