# Forward PBR 飞船接入延迟渲染场景 —— 技术发现报告

> 记录将 forward PBR 渲染的 GLB 飞船（`paris_class_heavy_frigate_-_halo_4k.glb`）叠加进 deferred shading + IBL + SSAO + Bloom 太空场景的完整过程，以及过程中发现的可复用技术要点与常见陷阱。

---

## 目录

1. [背景](#1-背景)
2. [重定义问题与公共类型抽取](#2-重定义问题与公共类型抽取)
3. [Forward / Deferred 管线与双重色调映射](#3-forward--deferred-管线与双重色调映射)
4. [金属暗面死黑与 spaceAmbient 填充](#4-金属暗面死黑与-spaceambient-填充)
5. [模型矩阵累积陷阱](#5-模型矩阵累积陷阱)
6. [阴影矩阵一致性](#6-阴影矩阵一致性)
7. [菲涅尔边缘光（Rim Light）](#7-菲涅尔边缘光rim-light)
8. [Bloom 与自发光](#8-bloom-与自发光)
9. [附录：关键代码](#9-附录关键代码)

---

## 1. 背景

场景 `SpaceShip11.0.cpp` 是 deferred 管线：G-Buffer → SSAO → 延迟光照 → 天空盒 → Bloom → Composite（Uncharted2 色调映射 + gamma）。行星/小行星均走 G-Buffer 写入。

飞船在 `PBR_6.0.cpp` 中是 **forward PBR** 直渲（直接算光照输出颜色）。二者管线不同，不能简单"换类加载"，需把 forward 渲染叠到延迟场景的 hdrFBO 上。

本文记录该接入过程中，解决 5 个问题、以及最终实现 rim light 与发光效果的技术要点。

---

## 2. 重定义问题与公共类型抽取

### 现象

`SpaceShip11.0.cpp` 同时 `#include "Model.h"`（行星/小行星）与 `#include "PbrModel.h"`（飞船）时，编译报 `redefinition of 'Vertex' / 'Texture'`。

### 成因

`Mesh.h` 与 `PbrMesh.h` 各自用 `#pragma once` 保护，但**两文件内部分别定义了同名全局 `struct Vertex` / `struct Texture`**（字段、顺序完全一致）。`#pragma once` 只防"同一文件被重复包含"，防不了"两个不同文件里的同名全局符号"。`class Model` / `class PbrModel` 名字不同，反而不冲突——冲突的只是那两个全局结构体。

### 解决

抽取公共类型到 `MeshTypes.h`，两个头文件改为 include：

```cpp
// MeshTypes.h
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glfw3.h>
#include <string>
#include <vector>
#define MAX_BONE_INFLUENCE 4

struct Vertex { glm::vec3 Position; glm::vec3 Normal; glm::vec2 TexCoords;
                glm::vec3 Tangent; glm::vec3 Bitangent;
                int BoneIDs[MAX_BONE_INFLUENCE]; float Weights[MAX_BONE_INFLUENCE]; };
struct Texture { unsigned int id; std::string type; std::string path; };
```

```cpp
// Mesh.h / PbrMesh.h：删掉各自的 struct 定义，改为
#include "MeshTypes.h"
```

### 可复用要点

> 多个头文件需要共享同一批类型时，抽到单独的 `*Types.h`，用 `#pragma once` 保护，其余头文件 include 它。**不要在两个头文件里各自定义同名的全局结构体**——`#pragma once` 救不了这种情况。

---

## 3. Forward / Deferred 管线与双重色调映射

### 现象

飞船原样搬进场景后，颜色发灰、发暗、且丢失 HDR 动态范围。

### 成因

`#pbr_frag6.0.shader`（飞船原 forward shader）**末尾自带 Reinhard 色调映射 + gamma**：

```glsl
color = color / (color + vec3(1.0));     // Reinhard
color = pow(color, vec3(1.0/2.2));       // gamma
```

而延迟场景的管线是：hdrFBO（存线性 HDR）→ Bloom → **Composite（Uncharted2 + gamma）**。飞船若把已经 tonemap 过的颜色写进 hdrFBO，Composite 会**再压一次**——双重色调映射，颜色发灰、暗部被压死、HDR 丢失。

### 解决

新建 `spaceship_frag.shader`，**去掉末尾 tonemap + gamma，直接输出线性 HDR**，交给 Composite 统一处理：

```glsl
// 删除这两行，只输出线性 HDR
// color = color / (color + vec3(1.0));
// color = pow(color, vec3(1.0/2.2));
FragColor = vec4(color, 1.0);
```

### 可复用要点

> **任何 forward 直渲物体要叠进"存 HDR + 统一 tonemap"的管线时，shader 必须输出线性 HDR，绝不能自带 tonemap/gamma。** 色调映射只在最终 Composite 做一次。

---

## 4. 金属暗面死黑与 spaceAmbient 填充

### 现象

飞船背光面（暗面）几乎全黑，轮廓不可辨；而同一场景里的行星暗面是可读的深蓝。两者用同一套 `spaceAmbient` 填充。

### 成因

延迟光照 shader 和飞船 shader 里都有一层"极微弱星野环境光"填充暗面：

```glsl
ambient += spaceAmbient * albedo * (1.0 - metallic);
```

行星/小行星是**非金属**（metallic = 0），`(1.0 - metallic) = 1.0`，填充完全生效。但飞船船壳是**金属**（metallic ≈ 1），`(1.0 - metallic) ≈ 0`——**这层填充光对金属几乎失效**。

而金属的暗面本就只能靠镜面反射（prefilteredColor）补光，太空 HDR 的背光方向 prefilter 又趋近 0 → 金属暗面死黑。

### 解决

飞船 shader 里去掉金属门控：

```glsl
ambient += spaceAmbient * albedo;   // 去掉 (1.0 - metallic)，金属也能吃到填充光
```

### 可复用要点

> **`*(1.0 - metallic)` 门控只适用于非金属的漫反射补光；金属物体（metallic≈1）的漫反射补光会被它归零。** 给金属补暗面，要么去掉门控（廉价艺术填充），要么走镜面反射下限（更物理）。

---

## 5. 模型矩阵累积陷阱

### 现象

修改 `spaceshipPosition` 后飞船位置几乎无变化，飞船缩在原点附近成一个极小的点。

### 成因

模型矩阵的基矩阵误写成非单位矩阵：

```glsl
glm::mat4 spaceshipModel = glm::mat4(0.0005f);   // 错误！对角全是 0.0005
spaceshipModel = glm::translate(spaceshipModel, spaceshipPosition);
...
spaceshipModel = glm::scale(spaceshipModel, glm::vec3(4.0f));
```

`glm::mat4(0.0005f)` 构造的是**对角线全 0.0005 的均匀缩放矩阵**，它排在乘法最前面，把**后面所有内容都乘 0.0005——包括 translate 的位移**：

```
最终位移 = 0.0005 × spaceshipPosition ≈ (0.0175, 0.006, 0.03)   // 几乎没动
最终缩放 = 0.0005 × 4.0        = 0.002                           // 缩成一个点
```

### 解决

基矩阵必须是单位矩阵，缩放只作用于模型本身：

```glsl
glm::mat4 spaceshipModel = glm::mat4(1.0f);                       // 单位矩阵
spaceshipModel = glm::translate(spaceshipModel, spaceshipPosition);
spaceshipModel = glm::rotate(spaceshipModel, glm::radians(-90.0f), glm::vec3(1,0,0));
spaceshipModel = glm::scale(spaceshipModel, glm::vec3(4.0f));     // 缩放单独作用
```

### 可复用要点

> **模型矩阵必须从 `glm::mat4(1.0f)` 起步。** `glm::mat4(x)` 是"对角线为 x 的缩放矩阵"，不是单位矩阵——用它做基矩阵会把平移一起缩掉。想缩小模型就改 `glm::scale` 的系数，别动基矩阵。

---

## 6. 阴影矩阵一致性

### 现象

飞船的阴影和本体位置错位（影子在某处，船在别处）。

### 成因

飞船要接收/投射阴影，必须在**阴影 pass**（写入 depth cubemap）和 **forward pass**（画本体）用**完全相同的 model 矩阵**。若两处各自构建矩阵，一旦顺序/参数不同，影子与本体就会分离。

### 解决

每帧**只构建一次** `spaceshipModel`（在循环开头），阴影 pass 和 forward pass 共用同一变量：

```cpp
// 循环开头构建一次
spaceshipModel = glm::mat4(1.0f);
spaceshipModel = glm::translate(...);
spaceshipModel = glm::rotate(...);
spaceshipModel = glm::scale(...);

// 阴影 pass
simpleDepthShader.setMat4("model", spaceshipModel);
spaceship.Draw(simpleDepthShader);

// forward pass
spaceshipShader.setMat4("model", spaceshipModel);
spaceship.Draw(spaceshipShader);
```

### 可复用要点

> **投射阴影的物体，其 model 矩阵在深度 pass 与颜色 pass 必须一致。** 最稳妥做法：一处构建，多处引用，绝不在两个 pass 里各自重复构建（否则易因顺序/参数不一致而错位）。

---

## 7. 菲涅尔边缘光（Rim Light）

### 技术

科幻飞船经典做法——在背光/剪影处勾勒一圈淡蓝色边缘光，强化轮廓。核心是**菲涅尔边缘项**：

```glsl
float rim = pow(1.0 - max(dot(N, V), 0.0), rimPower);
color += rimColor * rim * rimStrength;
```

- `N` = 表面法线，`V` = 视线方向
- 表面正对相机时 `dot(N,V)≈1`，`rim≈0`（中心无光）；掠射角时 `dot(N,V)→0`，`rim→1`（边缘最强）
- `rimPower` 控制边缘锐利度：越大边缘越细越锐

### 参数

| 参数 | 值 | 说明 |
|------|-----|------|
| `rimColor` | `(0.4, 0.6, 1.0)` | 淡蓝，与深蓝金属壳协调 |
| `rimPower` | `3.0` | 中等锐利度，刀形舰建议 3~4 |
| `rimStrength` | `0.6` | 强度，可 0.4~1.0 调 |

### 发光技巧（HDR rimColor）

飞船是线性 HDR 输出，`rimColor` 蓝色通道可 >1.0：

```glsl
// rimColor = vec3(0.5, 0.8, 1.2);  // 蓝通道 >1 → 过 Uncharted2 时产生辉光感
```

`>1.0` 的 HDR 值经过 Composite 的 Uncharted2 色调映射后会被压缩到 0~1 并保留高光层次，配合 Bloom 会产生"发光轮廓"的观感。

### 可复用要点

> **`pow(1.0 - dot(N,V), power)` 是最廉价的边缘光公式**，不依赖任何光照、只需法线和视线方向，适合给任意物体加剪影轮廓光。想更物理，可用 Fresnel-Schlick 项替代 `pow(1-dot(N,V),power)`。

---

## 8. Bloom 与自发光

### 当前配置

Composite 阶段的 Bloom 参数（`SpaceShip11.0.cpp`）：

| 参数 | 值 | 说明 |
|------|-----|------|
| `threshold` | `1.2` | 亮度阈值，>1.2 的 HDR 像素才被提取做泛光 |
| `bloomStrength` | `0.6` | 泛光混合强度 |
| `exposure` | `1.0` | 曝光 |
| `emissiveStrength`（飞船）| `2.0` | 飞船自发光贴图强度 |

### 原理

Bloom 流程：hdrColorBuffer 亮度提取（阈值 1.2）→ 乒乓高斯模糊（10 次）→ Composite 叠加。

只有**超过阈值 1.2 的 HDR 像素**才会被提取进泛光。因此：

- 普通被照亮的表面（<1.2）不产生泛光
- 自发光贴图 × `emissiveStrength`、或 >1.0 的 rim light，会突破阈值 → 泛光

### 让飞船发光点更"亮爆"的做法

1. **提高 `emissiveStrength`**：飞船发射窗/引擎的发光贴图强度（当前 2.0，可试 3~5）。
2. **降低 `threshold`**：让更多亮部进入泛光（如 0.9~1.0）。
3. **提高 `bloomStrength`**：整体泛光更浓。
4. **发射贴图通道 >1.0**：把 emission 贴图采样值乘更大的系数，产生 HDR 高光，喂给 bloom。

### 可复用要点

> **Bloom 的"发光"只对超过阈值的高亮 HDR 像素生效。** 想让某处发光，就要让它输出 >threshold 的 HDR 值（自发光强度、HDR 边缘光），而不是靠事后调亮度。

---

## 9. 附录：关键代码

### 9.1 飞船 shader 关键片段（`spaceship_frag.shader`）

```glsl
// 输入
vec3 N = getNormalFromMap();
vec3 V = normalize(camPos - WorldPos);

// IBL 环境光（金属暗面填充，已去金属门控）
vec3 ambient = (kD * diffuse + specular) * ao;
vec3 spaceAmbient = vec3(0.04, 0.06, 0.10);
ambient += spaceAmbient * albedo;               // 金属也能吃到

// 菲涅尔边缘光
float rim = pow(1.0 - max(dot(N, V), 0.0), rimPower);
vec3 color = ambient + Lo;
color += rimColor * rim * rimStrength;          // rimColor/power/strength 为 uniform

// 输出线性 HDR（无 tonemap，交给 Composite）
FragColor = vec4(color, 1.0);
```

### 9.2 太阳点光源（场景与飞船共用）

```cpp
glm::vec3 pointSunPositions = glm::vec3(-50.0f, 50.0f, -600.0f);
// color = (200, 200, 160)
// constant = 1.0, linear = 0.0002, quadratic = 0.000005
// far_plane = 800.0
```

### 9.3 阴影矩阵一致性（`SpaceShip11.0.cpp`）

```cpp
// 循环开头构建一次
spaceshipModel = glm::mat4(1.0f);
spaceshipModel = glm::translate(spaceshipModel, spaceshipPosition);
spaceshipModel = glm::rotate(spaceshipModel, glm::radians(-90.0f), glm::vec3(1,0,0));
spaceshipModel = glm::scale(spaceshipModel, glm::vec3(4.0f));
// 阴影 pass 与 forward pass 共用 spaceshipModel
```

---

*文档记录于 GLB 飞船接入延迟渲染场景的开发过程中，用于保留 forward/deferred 混合渲染、金属暗面补光、边缘光等可复用技术手段。*
