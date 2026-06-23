# 游戏项目中期检查报告

## 项目：深空星海——基于 OpenGL 的实时 3D 太空场景渲染

---

### 一、游戏项目说明

| 项目 | 说明 |
|------|------|
| **游戏名称** | 深空星海（Space Galaxy — 3D Space Scene） |
| **游戏类型** | 3D 太空场景探索 / 实时渲染 Demo |
| **核心玩法** | 玩家以第一人称视角自由漫游于深空场景中，观察由 5 万颗 GPU 实例化小行星组成的环形小行星带、一颗脉动恒星（含核心/日冕/辉光三层渲染）、一颗纹理行星，以及立方体贴图星空背景。当前阶段侧重渲染技术验证与场景搭建，为后续加入飞船驾驶、空间站交互等玩法奠定基础。 |
| **目标平台** | Windows (x64)，基于 OpenGL 3.3 Core Profile |
| **开发工具** | Visual Studio 2022 + C++ |
| **依赖库** | GLFW 3.x（窗口管理）、GLAD（OpenGL 加载器）、GLM（数学库）、Assimp（模型导入）、stb_image（纹理加载） |
| **开发模式** | 个人独立开发 |

---

### 二、推进计划回顾

项目从 OpenGL 基础学习开始，逐步过渡到综合场景搭建。原始里程碑规划如下：

| 阶段 | 时间（周） | 计划内容 |
|------|-----------|----------|
| **第一阶段** | 第 1–2 周 | 需求分析：确定项目主题为"太空场景"。搭建 OpenGL 开发环境，学习基础渲染管线（窗口创建、三角形绘制、纹理映射、坐标变换）。 |
| **第二阶段** | 第 3–4 周 | 原型设计：实现基础光照模型（Phong 模型、多光源、光源类型）、3D 模型加载（Assimp 集成）、封装 Shader / Model / Mesh 工具类。 |
| **第三阶段** | 第 5–6 周 | 核心机制实现：天空盒（Cubemap）、GPU 实例化（Instancing）、程序化天体生成（球体几何）、高级光照（法线贴图、阴影映射）。 |
| **第四阶段** | 第 7–8 周 | 场景整合：将各模块组装为完整的太空场景（恒星 + 行星 + 小行星带 + 星空背景），实现自由相机控制。 |
| **第五阶段** | 第 9–10 周 | 美术资源整合与调优：纹理替换、后处理（HDR/Bloom）、MSAA 抗锯齿、性能调优。 |
| **第六阶段** | 第 11–12 周 | 测试与交付：全面测试、Bug 修复、README 文档、GitHub 仓库整理、打包发布。 |

---

### 三、执行情况对比

#### 3.1 已完成功能清单（截至中期）

##### 模块 A：自建渲染基础设施（C++ 工具类）

编写了完整的 OpenGL 渲染辅助类库，总计约 **600+ 行**自制 C++ 代码：

**（1）Shader 着色器类** ([Shader.h](src/Shader.h) / [Shader.cpp](src/Shader.cpp))

- 从文件读取 GLSL 源码并编译顶点/片元/几何着色器
- 自动链接着色器程序并提供编译/链接错误诊断
- 封装 15+ Uniform 设置函数（`setBool / setInt / setFloat / setVec2-4 / setMat2-4`）

```cpp
// Shader 使用示例 - SpaceShip3.0.cpp: L91-L98
Shader planetshader("res/shader/InstancingShader/instancingVER.shader",
                    "res/shader/InstancingShader/instancingFRAG.shader");
Shader sunCoreShader("res/shader/StarShader/StarList2.0/core_ver.shader",
                     "res/shader/StarShader/StarList2.0/core_frag.shader");
// ... 共加载 8 个 Shader 程序
```

**（2）Model 模型加载类** ([Model.h](src/Model.h) / [Model.cpp](src/Model.cpp))

- 基于 Assimp 库加载 .obj / .fbx 等 3D 模型文件
- 递归处理场景节点树，提取 Mesh 数据（顶点、法线、纹理坐标、切线/副切线）
- 自动加载并去重材质纹理（漫反射、高光、法线、高度贴图）

```cpp
// Model 加载 - SpaceShip3.0.cpp: L102-L103
Model rock("res/model/rock/rock.obj");   // 小行星模型
Model planet("res/model/planet/planet.obj"); // 行星模型
```

**（3）Mesh 网格类** ([Mesh.h](src/Mesh.h) / [Mesh.cpp](src/Mesh.cpp))

- 管理 VAO/VBO/EBO 的生成与顶点属性配置
- 支持完整的 PBR 顶点结构（位置、法线、纹理坐标、切线、副切线、骨骼 ID 与权重）
- Draw 函数自动绑定纹理到对应的着色器采样器

**（4）Camera_ver2 四元数相机类** ([camera_ver2.h](src/camera_ver2.h) / [Camera_ver2.cpp](src/Camera_ver2.cpp))

- 使用**四元数（Quaternion）**实现万向节锁定免疫的 FPS 相机旋转
- 支持 WASD 键盘移动、鼠标视角控制、滚轮缩放（FOV 调节）
- 可通过方向键动态调整鼠标灵敏度（范围钳制在 0.01 ~ 0.5）

```cpp
// 相机使用 - SpaceShip3.0.cpp: L34
Camera_ver2 camera(glm::vec3(-40.0f, 10.0f, 200.0f));
// 控制逻辑 - SpaceShip3.0.cpp: L700-L739
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    // ... WASD 移动、鼠标控制、TAB 切换光标锁定
}
```

---

##### 模块 B：太空场景渲染系统

**（5）星空天空盒**（SpaceShip3.0.cpp: L294-L429）

- 使用 6 面立方体贴图（SpaceFox 1024×1024 纹理集）构建星空背景
- 天空盒始终以相机为中心（移除视图矩阵的平移分量），确保背景无限远

**（6）程序化恒星（太阳）渲染**（SpaceShip3.0.cpp: L196-L570）

- 纯代码生成球体网格（64×64 分段，基于球坐标参数方程）
- **三层渲染架构**，精心管理深度写入与混合状态：

| 图层 | 深度写入 | 深度比较 | 混合模式 | 渲染效果 |
|------|---------|---------|---------|---------|
| 核心层 (Core) | GL_TRUE | GL_LESS | 关闭 | 不透明球体，脉动缩放 + Y 轴旋转 |
| 日冕层 (Corona) | GL_FALSE | GL_LEQUAL | 加法混合 (GL_ONE) | 球体日冕 + 始终面向相机的广告牌光晕 |
| 辉光层 (Glow) | GL_FALSE | GL_LEQUAL | 加法混合 (GL_ONE) | 大范围柔软辉光，旋转 + 脉动 |

- 支持动态效果：`sin(time)` 驱动的脉动缩放、亮度振荡、旋转动画
- 渲染前后保存/恢复 OpenGL 状态（深度测试、混合、深度函数、深度掩码）

```cpp
// 恒星三层渲染核心代码 - SpaceShip3.0.cpp: L460-L570
// 1. 核心层（不透明）
glDepthMask(GL_TRUE); glDisable(GL_BLEND); glDepthFunc(GL_LESS);
sunCoreShader.use();
// ... 设置 model/view/projection，绘制球体
glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);

// 2. 日冕层（加法混合，不写深度）
glDepthMask(GL_FALSE); glDepthFunc(GL_LEQUAL); glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE);
// ... 绘制广告牌四边形 + 球体日冕

// 3. 辉光层（大范围柔光）
sunGlowShader.use();
// ... 绘制辉光球体
```

**（7）GPU 实例化 — 50,000 颗小行星带**（SpaceShip3.0.cpp: L107-L192, L620-L655）

- 使用 `glDrawElementsInstanced` 一次性提交 50,000 颗小行星
- 每个小行星的 4×4 模型矩阵作为实例化顶点属性（4 个 vec4，每实例步进一次）
- 随机生成算法：
  - 环形分布（角度均匀分配 + 径向随机偏移 ±20 单位，半径 200）
  - 4 级尺寸变化（每第 5/20/200 个放大 1.5×/3×/6×，其余随机 0.01–0.30）
  - 3 轴独立随机旋转角度

```cpp
// GPU 实例化核心代码 - SpaceShip3.0.cpp: L109-L190
unsigned int amount = 50000;
glm::mat4* modelMatrices = new glm::mat4[amount];
for (unsigned int i = 0; i < amount; i++) {
    // 1. 环形分布 + 随机位移
    float angle = (float)i / (float)amount * 360.0f;
    float x = sin(angle) * radius + displacement;
    float z = cos(angle) * radius + displacement;
    // 2. 随机缩放（4 级尺寸）
    float scale = ...;
    if (i % 200 == 0) scale *= 6.0f;
    else if (i % 20 == 0) scale *= 3.0f;
    else if (i % 5 == 0) scale *= 1.5f;
    // 3. 三轴随机旋转
    // 4. 存入矩阵数组
}
// VBO 上传 → 顶点属性配置 → glDrawElementsInstanced
```

**（8）光照系统**（SpaceShip3.0.cpp: L591-L655）

- **定向光**（Directional Light）：环境光 0.05 + 漫反射 0.4 + 镜面反射 0.5
- **点光源**（Point Light from Star）：从恒星位置发出，环境光 1.0 + 漫反射 200.0 + 镜面反射 120.0（超高亮度模拟恒星辐射）
- 光照衰减参数：常数项 1.0、线性项 0.0002、二次项 0.000005（远距离衰减极慢，模拟太空环境）

---

##### 模块 C：交互系统

**（9）自由相机控制**（SpaceShip3.0.cpp: L700-L739）

- WASD 键：前后左右移动（速度 × deltaTime 帧率无关）
- 鼠标：四元数旋转视角（水平/俯仰）
- 滚轮：缩放视野（FOV 1° ~ 45°）
- 方向键 ↑/↓：调节鼠标灵敏度（0.01 ~ 0.5）
- TAB 键：切换光标锁定 / 释放（方便调试）

**（10）渲染质量**

- 4× MSAA（多重采样抗锯齿）(L56, L88)
- 动态窗口比例适配（`glfwGetFramebufferSize` 获取实际窗口大小）

---

#### 3.2 与计划的偏差分析

| 计划项 | 状态 | 偏差说明 |
|--------|------|----------|
| 第一阶段：环境搭建与基础学习 | ✅ 超前完成 | 从 `src/` 目录可见，已完成 60+ 个练习源文件（`01_BasicOpenGL` → `05_AdvancedLighting`），远超计划范围。 |
| 第二阶段：着色器/模型工具类 | ✅ 按期完成 | `Shader.h/cpp`、`Model.h/cpp`、`Mesh.h/cpp` 均已封装完毕并稳定运行。 |
| 第三阶段：核心渲染技术 | ✅ 按期完成 | 天空盒、GPU 实例化（50K 小行星）、程序化球体生成、恒星三层渲染均已完成。 |
| 第四阶段：场景整合 | ✅ 按期完成 | `SpaceShip3.0.cpp` 已将所有模块整合为完整的可运行太空场景。 |
| 第五阶段：后处理 (HDR/Bloom) | 🔶 进行中 | `SpaceShip4.0.cpp` 已开始搭建帧缓冲框架（`setupFramebuffers` 函数骨架），但当前代码已被整体注释，说明正在重构或调试中。 |
| 第六阶段：测试/GitHub/交付 | ❌ 未开始 | GitHub 仓库尚未创建，README 未编写，系统测试未进行。 |

**超前原因分析：**
- 对 OpenGL 渲染技术兴趣浓厚，投入了远超课程要求的学习时间
- 网上教程资源（LearnOpenGL 等）结构清晰，降低了自学的门槛
- 使用的 C++/OpenGL 技术栈在 B 站和 GitHub 上有较丰富的参考案例

**滞后原因分析：**
- 后处理（HDR/Bloom）技术复杂度较高，涉及多通道渲染和帧缓冲管理
- 个人独立开发，所有编码、调试、资源搜集均需自行完成，时间分配紧张
- GitHub 使用经验不足，版本管理尚未纳入工作流

---

#### 3.3 遇到的困难与解决措施

| 困难类型 | 具体描述 | 解决措施 |
|----------|---------|---------|
| **技术：深度/混合状态管理** | 恒星三层渲染需要不同的深度写入、深度比较和混合模式，状态切换不当会导致图层遮挡错误或渲染顺序错乱。 | 在渲染前后显式保存/恢复 OpenGL 状态（`glGetBooleanv / glGetIntegerv`），逐层精确设置 `glDepthMask`、`glDepthFunc`、`glBlendFunc`，确保层间互不干扰。详见 SpaceShip3.0.cpp: L450-L583。 |
| **技术：日冕广告牌旋转** | 日冕四边形需要始终面向相机（Billboarding），但使用 `glm::lookAt` 直接计算旋转矩阵时出现方向反转问题。 | 使用 `glm::inverse(glm::lookAt(...)) ` 取逆矩阵来修正旋转方向（L505-L506），使四边形始终正对相机。 |
| **技术：四元数相机万向节锁定** | 使用欧拉角实现 FPS 相机时，俯仰角达到 ±90° 会产生万向节锁定，视角翻转异常。 | 重构相机系统，采用四元数（`glm::quat`）存储相机朝向，通过 `angleAxis` 增量旋转避免了万向节锁定问题（Camera_ver2.cpp: L43-L61）。 |
| **技术：50,000 颗小行星性能** | 普通逐个 Draw Call 无法承受 5 万对象的渲染压力，帧率极低。 | 采用 GPU 实例化技术（Instancing），将所有模型矩阵上传至 VBO，使用 `glDrawElementsInstanced` 一次 Draw Call 渲染全部小行星，帧率稳定在 60 FPS 以上。 |
| **团队协作：个人开发的局限** | 独立开发缺乏代码审查和设计讨论，部分代码结构不够清晰（如 `SpaceShip3.0.cpp` 880 行全写在 main 中）。 | 认识到问题——计划在 `SpaceShip4.0` 中将恒星渲染、小行星带、后处理等模块拆分为独立的类/函数，提升代码可维护性。 |
| **时间管理** | 沉浸于技术细节（如反复调整恒星脉动参数），挤占了项目管理和文档编写时间。 | 制定明确的功能优先级，先完成核心场景（已达成），后期集中处理文档和仓库管理。 |

---

### 四、代码文件清单

#### 4.1 自编源文件总览

```
src/
├── SpaceShip3.0.cpp       ← 当前主场景（880 行，星空 + 恒星 + 行星 + 5 万小行星）
├── SpaceShip4.0.cpp       ← 下一版本（已注释，正在添加 HDR/Bloom 帧缓冲）
├── Shader.h / Shader.cpp   ← 着色器加载、编译、链接、Uniform 工具类
├── Model.h / Model.cpp     ← Assimp 模型加载、材质纹理管理
├── Mesh.h / Mesh.cpp       ← 网格 VAO/VBO/EBO 管理与绘制
├── Camera_ver2.h / .cpp    ← 四元数 FPS 自由相机
│
├── 01_BasicOpenGL/         ← 第 1–2 周：基础（6 个练习）
├── 02_BasicLight/          ← 第 3 周：光照（5 个练习）
├── 03_Model/               ← 第 4 周：模型加载（3 个练习）
├── 04_AdvancedOpenGL/      ← 第 5–6 周：高级 OpenGL（实例化、帧缓冲、几何着色器等）
├── 05_AdvancedLighting/    ← 第 7 周：高级光照（法线贴图、阴影映射、HDR）
└── SkyBox/                 ← 天空盒专项练习
```

#### 4.2 着色器资源（自编 GLSL）

```
res/shader/
├── InstancingShader/       ← 实例化渲染着色器（小行星带使用）
├── StarShader/StarList2.0/ ← 恒星三层渲染着色器（核心/日冕/辉光）
├── SkyBoxShader/           ← 天空盒着色器
├── ModelShader/            ← 模型光照着色器
└── ...                     ← 其余学习阶段着色器
```

---

### 五、后续计划

#### 5.1 剩余功能点

| 优先级 | 功能 | 预计耗时 | 描述 |
|--------|------|---------|------|
| 🔴 高 | HDR + Bloom 后处理 | 1 周 | 在 `SpaceShip4.0` 中完成帧缓冲链路：HDR 渲染 → 高斯模糊（两趟乒乓缓冲）→ 叠加合成，实现恒星辉光的 Bloom 效果。 |
| 🔴 高 | 代码重构 | 0.5 周 | 将 880 行的 `main()` 拆分为 `StarRenderer`、`AsteroidBelt`、`SkyboxRenderer` 等独立模块。 |
| 🟡 中 | 飞船驾驶系统 | 1.5 周 | 添加飞船模型，实现惯性飞行、加减速、翻滚控制（区别于当前的自由相机模式）。 |
| 🟡 中 | UI/HUD 界面 | 0.5 周 | 使用 ImGui 添加速度表、坐标显示、帧率显示等 HUD 信息。 |
| 🟢 低 | 场景交互 | 1 周 | 实现小行星碰撞检测、空间站对接等交互玩法。 |
| 🟢 低 | GitHub 仓库整理 | 0.5 周 | 创建仓库、编写 README、整理提交历史、添加 .gitignore。 |

#### 5.2 风险预测

| 风险项 | 概率 | 影响 | 应对措施 |
|--------|------|------|----------|
| HDR/Bloom 实现不符合预期（画面过曝或模糊过度） | 中 | 画面质量下降 | 参考 LearnOpenGL 的 Bloom 章节，逐步调参；预留多个参数调节接口。 |
| 重构引入新 Bug | 中 | 影响已有功能稳定性 | 重构前在 `SpaceShip3.0` 基础上创建分支，保留可回退版本。 |
| 时间不足（学期剩余约 4–6 周） | 中高 | 部分计划功能无法完成 | 优先保证 HDR/Bloom + 代码重构，飞船驾驶和 UI 作为可选加分项。 |
| GitHub 使用不熟练 | 中 | 仓库管理扣分 | 尽早创建仓库并开始上传，前期允许提交记录不完美，重点是有可读的 README 和历史记录。 |

#### 5.3 测试方案

| 测试类型 | 内容 |
|----------|------|
| **功能测试** | 验证恒星三层渲染正确性、小行星实例化数量与分布、光照参数合理性、相机控制无异常。 |
| **性能测试** | 使用 RenderDoc / GPU Profiler 测量 Draw Call 数量、GPU 占用率，确保 50K 小行星场景帧率 ≥ 60 FPS。 |
| **兼容性测试** | 在不同 NVIDIA/AMD 显卡上测试（主要宿舍电脑），确保 OpenGL 3.3 Core 兼容性。 |
| **回归测试** | 重构后逐功能对比 `SpaceShip3.0` 与 `SpaceShip4.0` 的渲染输出，确保视觉效果一致。 |

#### 5.4 最终交付时间

- **最终交付日期**：学期第 12 周（约 2026 年 7 月中旬）
- **交付物**：
  1. 可执行程序（Release 版本）+ 源代码
  2. GitHub 仓库链接（含 README 说明文档）
  3. 中期检查报告 PDF（本文档）

---

### 六、项目当前效果描述

启动 `SpaceShip3.0` 后，用户将看到一个深邃的太空场景：

1. **远景**：由 6 面 SpaceFox 纹理组成的立方体贴图星空背景，环绕整个场景。
2. **中景**：一颗温暖橙黄色调的恒星悬于左前方远处（坐标 -50, 50, -600），占据视野的近 1/3。恒星呈现三层视觉效果——明亮的核心球体、半透明的日冕光晕、以及大面积柔软辉光，整体随正弦波周期性脉动。
3. **近景**：一颗纹理行星（坐标 0, -3, 0）位于视野中央偏下方。
4. **环绕**：以恒星为中心，约 5 万颗大小不一、随机旋转的小行星组成巨大的环形小行星带，半径约 200 单位，分布带有随机扰动以形成自然感。

用户可通过 WASD 自由飞行穿越小行星带、靠近恒星观察其脉动细节、或远距离俯瞰整个场景。

---

*报告生成日期：2026 年 6 月 24 日*
*开发周期：第 1–8 周（中期节点）*
