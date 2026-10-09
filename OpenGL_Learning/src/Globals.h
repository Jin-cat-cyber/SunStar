#pragma once
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include "camera_ver2.h"
#include "Spaceship.h"
#include "WarpDebris.h"

// ===== 常量 =====
inline constexpr unsigned int SCR_WIDTH = 960;
inline constexpr unsigned int SCR_HEIGHT = 600;
inline constexpr unsigned int SHADOW_WIDTH = 4096;
inline constexpr unsigned int SHADOW_HEIGHT = 4096;
inline constexpr float		  PI = 3.14159265359f;

// ===== 窗口 =====
inline int windowwidth = SCR_WIDTH;
inline int windowheight = SCR_HEIGHT;


// ===== 相机 =====
inline Camera_ver2 camera(glm::vec3(-40.0f, 10.0f, 200.0f));
inline float lastX = SCR_WIDTH / 2.0f;
inline float lastY = SCR_HEIGHT / 2.0f;
inline bool  firstMouse = true;


// ===== 输入状态 / 开关 =====
inline bool cursorLocked = true;
inline bool tabKeyPressed = false;      // 用于检测 TAB 键的上升沿
inline bool f10Pressed = false;         // 用于检测 F10 键的上升沿
inline bool f11Pressed = false;         // 用于检测 F11 键的上升沿
inline bool isFullscreen = false;       // 是否全屏
inline bool shadows = true;
inline bool PCSS = false;
inline bool ssaoEnabled = true;
inline bool unify = true;
//  M 键：SMAA 总开关（调参阶段做 A/B 对比用）。关掉时整段跳过 —— 连两次 blit 都不做，
//  于是画面与"根本没接 SMAA"逐干净的对照。
inline bool smaaEnabled = true;
inline bool smaaKeyPressed = false;

inline bool shadowKeyPressed = false;
inline bool PCSSKeyPressed = false;
inline bool ssaoKeyPressed = false;
inline bool unifyKeyPressed = false;

inline int savedX = 0, savedY = 0;
inline int savedWidth = SCR_WIDTH, savedHeight = SCR_HEIGHT;


// ===== 时间 =====
inline float deltaTime = 0.0f;
inline float lastFrame = 0.0f;


// ===== 帧缓冲 =====
inline unsigned int hdrFBO = 0, hdrColorBuffer = 0, hdrDepthRBO = 0;
inline unsigned int hdrDepthTex = 0;   // hdrFBO 的深度附件（纹理版）：可采样，供冲击波扭曲 pass 反算世界坐标
inline unsigned int pingpongFBO[2] = { 0, 0 };
inline unsigned int pingpongColorbuffers[2] = { 0, 0 };
	/*G - buffer*/ 
inline unsigned int gBuffer = 0;
inline unsigned int	gPosition = 0;
inline unsigned int	gNormal = 0;
inline unsigned int	gAlbedo = 0;
inline unsigned int	gPBR = 0;
inline unsigned int gDepthRBO = 0;

//// ===== TAA：每帧时间维状态 =====
////  必须每帧显式快照：相机抖动只加到临时的 camera.Position 上（算完 view 立刻减回），
////  下一帧无法重新算出一份"当时的 view"，所以本帧与上一帧的 VP 都得存下来。
//struct TaaState
//{
//    glm::mat4 curView = glm::mat4(1.0f);
//    glm::mat4 prevView = glm::mat4(1.0f);
//    glm::mat4 curProj = glm::mat4(1.0f);
//    glm::mat4 prevProj = glm::mat4(1.0f);
//    float curTanHalfFov = 0.0f;   // 本帧【实际用的】tan(fov/2)：含 FOV 冲击，天空支路要用
//    float curAspect = 1.0f;
//    glm::vec3 prevCamPos = glm::vec3(0.0f);   // 上一帧相机位置：去遮挡检测要与"上一帧量出的距离"同源
//    glm::vec2 curJitter = glm::vec2(0.0f);
//    glm::vec2 prevJitter = glm::vec2(0.0f);
//    int frameIndex = 0;      // 抖动序列下标，取模 JITTER_N 使用
//    bool reset = true;       // 历史失效：首帧、窗口尺寸变化、折跃传送那一帧
//    int historyRead = 0;     // 历史乒乓：本帧读哪一张
//    int historyWrite = 1;    // 本帧写哪一张，帧末与 read 交换
//};
//inline TaaState gTaa;
//// ===== TAA：历史颜色（乒乓两张）=====
////  必须【渲染分辨率】+ GL_RGBA16F + GL_LINEAR：重投影后的 uvPrev 是任意小数坐标（要线性过滤），
////  而 TAA 在 tonemap 之前、颜色是线性 HDR（要 16F）。见计划书 §8.1。
//inline unsigned int taaHistoryFBO[2] = { 0, 0 };
//inline unsigned int taaHistoryColorBuffer[2] = { 0, 0 };


//  ===== SMAA（形态学抗锯齿）：两遍全屏 + 两次 blit，作用在【合成之后的 LDR 图】上 =====
//  为什么吃 LDR：阈值是按亮度定的绝对量（0.06 = 亮度差 6%），tonemap + gamma 之后的图才是
//  感知均匀的 [0,1]；混合也是感知混合（把 10.0 的高光混进 0.05 的暗部会读成"边缘发光"）。
//  为什么用 blit 进出默认帧缓冲：这样不必改共享的 PostProcess.{h,cpp}，合成照旧写默认帧缓冲。
inline unsigned int smaaFBO[2] = { 0, 0 };          // [0] = 输入副本，[1] = 混合输出
inline unsigned int smaaColorBuffer[2] = { 0, 0 };
inline unsigned int smaaEdgeFBO = 0;
inline unsigned int smaaEdgeTex = 0;

//  观感常量：要调就调这三个（写法与工程里 `const int samples = 4;` 一致）
inline constexpr float SMAA_THRESHOLD = 0.06f;       // 边缘阈值：亮度差超过它才算边。高了边不够、低了连纹理一起磨
inline constexpr float SMAA_LOCAL_CONTRAST = 0.15f;  // 局部对比自适应：亮处把阈值抬高；0 = 关闭
inline constexpr int   SMAA_MAX_STEPS = 8;           // 第二遍的搜索步数上限（主要开销旋钮）


// ===== PBR 离屏资源（inline：跨 TU 单实例）=====
inline unsigned int captureFBO = 0;
inline unsigned int captureRBO = 0;

// HDR 环境贴图
inline unsigned int hdrTexture = 0;
inline unsigned int envCubemap = 0;
// 辐照度
inline unsigned int irradianceMap = 0;
//
inline unsigned int prefilterMap = 0;
inline unsigned int brdfLUTTexture = 0;

inline glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
inline glm::mat4 captureViews[6] =
{
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, 1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,-1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
};

// ===== 阴影 =====
inline unsigned int depthCubeMap = 0, depthCubeFBO = 0;
inline float shadow_near = 1.0f;
inline float shadow_far = 1000.0f;
inline unsigned int shadowColorMap = 0;

// ===== 阴影：静态烘焙 + 动态层 =====
inline bool shadowStaticDirty = true;               // 置真 → 下帧重烘焙静态层
inline unsigned int depthDynFBO = 0;                // 动态层 FBO
inline unsigned int depthDynMap = 0;                // 动态层 cubemap (1024, 6面)
inline constexpr unsigned int DYN_SHADOW_SIZE = 1024;

// ===== 阴影：逐面剔除 =====
inline std::vector<glm::mat4> gFaceCasters[6];		      // 每个面自己的投射体
inline unsigned int			  gFaceCount[6] = { 0,0,0,0,0,0 };


// ===== SSAO =====
inline unsigned int ssaoFBO = 0, ssaoBlurFBO = 0;
inline unsigned int ssaoColorBuffer = 0, ssaoColorBufferBlur = 0;
inline std::vector<glm::vec3> ssaoKernel;
inline unsigned int noiseTexture = 0;

// ===== ship / view mode =====
inline Spaceship ship;
inline ViewMode currentMode = MODE_FREE;

// ===== third-person follow params =====
inline float followDistance = 80.0f;	// 相机在飞船后方的距离
inline float followHeight = 25.0f;		// 相机在飞船上方的高度
inline float followSmooth = 2.0f;		// 跟随阻尼（值越大跟随越紧，值越小越柔和），10 的感觉偏硬，换 5 会有滞后感
inline float orbitYaw = 0.0f;    // 模式-3 环视：水平环绕角
inline float orbitPitch = 0.0f;  // 模式-3 环视：垂直环绕角



// ===== 战术折跃（warp）状态 =====

//      纯视觉状态。它唯一落在物理侧的动作是那一次传送，且只在闪现权重恰为 1
//      的那一帧执行（见 Warp.h 的 advance 与计划书 §4）。
//      之所以不放进 Spaceship：物理侧只需要 WarpTo 一个方法，而相机与后处理
//      （FOV 冲击）都要读这里。
struct WarpState
{
    bool      active = false;               // 时间线正在运行
    //bool      teleported = false;               // 本帧执行传送
    // 理由：传送是单帧事件，标志由 warp::Step::teleport 返回，不放进跨帧状态。
    float     tau = 0.0f;                   // 自按键起算的秒数
    float     cooldown = 0.0f;              // 距下次可用还剩的秒数
    float     reject = 0.0f;                // 拒绝反馈红闪的剩余时间；与时间线无关
    // glm::vec3 delta = glm::vec3(0.0f);      // 折跃位移向量 D_eff * f_hat
    glm::vec3 target = glm::vec3(0.0f);     // 冻结的落点：按键帧定死，传送那一帧直接用它
};
inline WarpState gWarp;

inline float WARP_DISTANCE = 100.0f;
inline float WARP_COOLDOWN = 6.0f;
inline float WARP_MIN_DIST = 10.0f;
inline bool tKeyPressed    = false;
// 行星位置与世界半径：折跃落点合法性要用，而 T 键处理在 processInput 里、
// 拿不到 main 的局部量，所以在 main 里算完半径后回填一次。
inline glm::vec3 gPlanetPos = glm::vec3(0.0f, -3.0f, 0.0f);
inline float gPlanetRadius = 0.0f;
inline float gShipBowOffset = 0.0f;         // 舰首到模型原点的距离；折跃落点合法性要用
inline float WARP_REJECT_FLASH = 0.30f;     // 拒绝反馈红闪时长（秒）
inline float shipBoundR = 0.0f;            // 舰体包围球半径（阴影/剔除/折跃共用）
inline float shipAxisMinY = 0.0f;          // 模型局部 Y 最小值（机头在这一端）
inline float shipAxisMaxY = 0.0f;          // 模型局部 Y 最大值


// ===== 尘埃星环 =====
inline unsigned int ringVAO = 0, ringVBO = 0, ringEBO = 0;
inline unsigned int ringIndexCount = 0;
inline float ringInner = 150.0f;   // 内半径（占位，待调）
inline float ringOuter = 260.0f;   // 外半径（占位，待调）
inline float ringThickness = 15.0f;   // 环的厚度（±15 单位，很扁但可见）

// ===== 小行星视锥剔除（跨版本共用）=====
inline std::vector<glm::mat4> gRockMatrices;	// 小行星模型矩阵数组(全量矩阵，CPU端)，不再每帧计算
inline std::vector<glm::vec4> gRockSpheres;		// xyz=球心， w=半径
inline unsigned int rockInstanceVBO = 0;		// 实例矩阵动态 VBO 句柄
inline std::vector<glm::mat4> gVisible;			// 本帧可见小行星矩阵数组（CPU端）
inline unsigned int rockVisibleCount = 0;		// 本帧可见小行星绘制数量（CPU端）
inline std::vector<glm::mat4> gShadowVisible;	// 阴影 Pass 用（能投影到可见面的）
inline unsigned int rockShadowVisibleCount = 0; // 阴影 Pass 绘制数量

// ===== 小行星 LOD =====
inline std::vector<glm::mat4> gNear;		  // LOD0 近桶矩阵（全模）
inline std::vector<glm::vec4> gFarPos;		  // LOD1 远桶：xyz=位置, w=点径像素
inline unsigned int rockNearCount = 0;
inline unsigned int rockFarCount = 0;
inline std::vector<unsigned char> gLodLevel;  // 每实例当前档(0近/1远)，迟滞记忆
inline unsigned int rockPointVAO = 0;         // 点精灵 VAO
inline unsigned int rockPointVBO = 0;         // 点精灵 VBO
inline bool lodEnabled = false;				  // LOD 总开关：false = 全部走全模(画面等同 17.A)