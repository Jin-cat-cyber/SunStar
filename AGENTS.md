# OpenGL_Learning —— 项目约定与踩坑清单

> 给 AI 助手的持久上下文（DSH 每次会话自动加载）。
> **技术细节不写在这里** —— 数学/实现推导一律进 `Tech_finding_Report/`；本文件只放跨会话必须知道的
> **规矩、状态、教训**。
> 最后更新：2026-09-15

## 0. 项目速览
- VS 解决方案 `OpenGL_Learning.sln`；MSVC v143、C++17（`stdcpp17`）、`WholeProgramOptimization=true`。
- 目录：源码 `OpenGL_Learning/src/`；着色器 `OpenGL_Learning/res/shader/`；技术报告 `OpenGL_Learning/Tech_finding_Report/`。
- 管线：延迟着色（G-Buffer + 小行星实例化）、前向 PBR（火星、飞船）、点光源立方体阴影（per-face）、SSAO、IBL、尘埃星环体积光、行星大气、后处理（bloom / lens flare / tonemap / gamma）。
- 已抽成独立模块：`src/InitPBR.{h,cpp}`（IBL 初始化）、`src/Procedural.{h,cpp}`（剔除/几何工具）、`src/PostProcess.{h,cpp}`（后处理，接口 `PostProcessing(Shader& brightPass, Shader& blur, Shader& composite, const unsigned int& quadVAO)`）。

## 1. 版本约定（改动前必读）
- 多个 `SpaceShipNN.cpp` 并存；**活跃版本的 `#ifdef SHIP_x_x` 是注释掉的**（永远参与编译），其余版本门控保留（宏未定义 → 整段不编译）。
- **当前活跃版本：`src/SpaceShip21.0.cpp`**；`SpaceShip20.0.cpp` 的 `#ifdef SHIP_20_0` 未注释 = 休眠。
- 新版本 = 复制活跃版本再改。**不要动休眠版本**，更不要给休眠版本松开门控（会多出一个 `main` → 链接冲突）。

## 2. 协作规矩（硬性）
- **先读文件再下结论**，不要凭记忆或猜测给建议。
- **反馈 ≠ 指令**：用户描述画面/现象时，默认"上一条建议已被实施"，按新状态分析。
- **没有明确指令不改文件**：只给方案（意图 → 原理 → 具体改动 + 行号锚点），由用户自己改。
- **不清楚就先问**，别瞎猜、别兜圈子；也别顺手甩一堆没被问到的东西。
- **行动前先用文字说明要做什么**；建议未被确认时不要越级推进下一步。
- 用户默认接受方案（沉默 ≈ 同意），但不确定处必须主动、详细地提问。
- **偏好轻量方案**：没被要求时不要推类化 / 多文件大重构（Scene 类化尝试失败已删）。
- 简洁省 token，结论先行。
- 公式：终端/输出窗口不渲染 LaTeX —— 用源码字面量 + 白话解释，不要用 `$$` 定界符。

## 3. 代码规矩（本项目特有）
- 共享状态一律放 `src/Globals.h`，用 C++17 **`inline` 变量**。
- 头文件里**只放声明**：函数要么 `inline`、要么实现进 `.cpp`；变量要么 `inline`、要么 `extern`。
  - 类内定义的成员函数是**隐式 inline**，所以 `Spaceship.h` 那种"类里直接写函数体"是多 TU 安全的。
- `STB_IMAGE_IMPLEMENTATION` 全项目只允许定义一次（当前在 `Procedural.cpp`）；只用 `stbi_*` 的 TU 只 include 声明。
- 新增 `.cpp` **必须加进 `OpenGL_Learning/OpenGL_Learning.vcxproj`**：没加 → 别的 TU 调它会 LNK2019（未解析外部符号）；加重复条目 → LNK2005。**不要手动复制 .vcxproj 的行**，用 VS"添加现有项"。
- 同名函数重载时**不要给某个参加默认值** —— 会与少参版本产生 ambiguous。
- 对外接口参数用 `unsigned int` 而不是 `GLuint`，避免头文件依赖 glad（glad 必须在 GLFW 之前包含）。

## 4. 踩过的坑（每条指回报告）
| 教训（一句话） | 出处 |
|---|---|
| **包围球半径必须取 σ_max 量级**：均匀缩放×旋转时 = 任取一列长度；**绝不能用矩阵对角线元素** | `小行星包围球半径与阴影裁剪修复_数学原理技术报告.md` |
| 保守判据 + 正确包围体 ⇒ "部分可见却被剔除"不可能发生；遇到 pop **先查包围体**，别急着加 margin | 同上 |
| 光源可见性判据（C）必须带上球半径，否则视锥边缘投射者的阴影会**整块消失** | 同上 |
| 大气全屏 pass 必须写 `gl_FragDepth` + 开深度测试，否则会照亮小行星带 | 大气散射全屏Pass遮挡修复 |
| VSM 在 per-face + 几何着色器下颜色/深度不同步 → 已回退 PCF | 阴影单面渲染 |
| LOD 远景点精灵已废弃（pop + 收益不明显），代码留在 `lodEnabled=false` 后面 | 小行星LOD_尝试与废弃 |
| 阴影 bias 数值（飞船 40~80、星带 21、火星 0.15 + 法线偏移）是**实调值，别乱改** | — |
| 着色器里被注释掉的后处理 / 恒星体积光是**刻意设计**，不是遗漏 | — |
| `shadow_far = 1000` 是调过的（解决远处横切），改小会重现 | — |
| 双显卡笔记本要强制独显（Optimus），否则跑核显 | 01-dual-gpu-optimus |

## 5. 报告索引
全部报告在 `OpenGL_Learning/Tech_finding_Report/`。**改某个子系统前先翻对应报告**，别重复踩坑。
常被引用：包围球半径与阴影裁剪修复（数学原理）、小行星视锥剔除_方案C、阴影单面渲染_程序流、行星大气散射_数学推导、尘埃星环_数学原理、Specular_IBL_数学推导、渲染求值模型_表面与介质、第三人称跟随相机平滑阻尼。

## 6. 待办 / 路线图
- 小项：`InitPBR.h/.cpp` 宽高参数 `float → int`（顺手清注释块）；`SpaceShip21.0.cpp:27` 门控名 `SHIP_19_A` → `SHIP_21_0`；L829-902 旧后处理死代码可删。
- 临时自测块（建议统一包 `#ifdef DEBUG_CULL`）：`SpaceShip21.0.cpp:1335-1340`（`[radius]`）与 `:1651-1665`（`[cull]` / `[shadow]`）。半径修好后 `f = s / scale` 恒为 1.0，**`[radius]` 一旦打印就说明半径回归了**。
- 路线图（性能向，不急）：② 阴影近远精度拆分 → ④ 体积光降采样（1/2 或 1/4 + upsample）→ ③ 抗锯齿。

## 7. 维护约定
- 只记"跨会话要用"的规矩/状态/教训，一条一两行；数学与实现细节写进 `Tech_finding_Report/`，这里只留结论 + 指路。
- 状态变化（如活跃版本号）**改这一份**，不要追加重复条目。
- 判断标准：不可从代码推导、且对后续会话有用。

## 8. 运行环境（助手自身）
- 会话日志：`$DSH_SESSION_JSONL`（`session.jsonl.zstd`，**多帧 zstd**）。逐帧解压后可 grep `"type":"compaction` / `"type":"command/` 拿到全部事件（含每次命令的 `kind` 与 `text`）。
- `/compact`：**只在 agent 空闲且无活动压缩时可用，命令不排队** —— 连敲只会秒回 busy；大会话一次约 20 秒。
- 压缩收缩校验：摘要的估算 token 必须**小于**被替换范围，否则整次压缩放弃、对话保持不变（UI 显示 "could not produce a useful summary"）。
- 自动压缩阈值：上下文用量 80%（`thresholdRatio` 默认 0.8）；保留比 `retainRatio` 默认 0.16。
