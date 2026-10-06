#include <glad/glad.h>
#include <glfw3.h>
#include <assimp/config.h>
#include <assimp/revision.h>
#include <random>

#include "Globals.h"

#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera_ver2.h"
#include "Model.h"
#include "PbrModel.h"
#include "Shader.h" // 包含自定义着色器类
#include "Sun.h"
#include "Skybox.h"
#include "Procedural.h"
#include "Spaceship.h"
#include "InitPBR.h"
#include "PostProcess.h"
#include "PhysicsSelfTest.h"
#include "WarpSC.h"
#include "WarpDebris.h"
#include "WarpSelfTest.h"
#include "WarpSCSelfTest.h"
#include "WarpTuning.h"
#include "WarpPillar.h"
#include "WarpShock.h"

#ifdef SHIP_24_D
#include <stb_image.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);  // 窗口大小回调函数
void processInput(GLFWwindow* window);  // 输入检查函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos);  // 鼠标 移动 回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);   // 鼠标 滚轮 回调函数
void setupFramebuffers(int eidth, int height);  //  离屏渲染帧缓冲
void rebuildFramebuffers(int width, int height);  //  重建离屏渲染帧缓冲



// 星球相关函数
void RocksModelMatricesInit(unsigned int& amount, Model& rock);
void RockViewFrustumCull(GLFWwindow* window, const glm::vec3& lightPos);

// 阴影相关函数
void DepthCubeMapInit();
void ShadowPassRender(glm::mat4& shadowProj, std::vector<glm::mat4>& shadowTransforms, const glm::vec3& pointSunPositions);
int StaticShadowFace(const glm::vec3& lightPos, const glm::vec3& center);
void ShadowPassBegin(unsigned int fbo, int res, glm::mat4& shadowProj,
    std::vector<glm::mat4>& shadowTransforms, const glm::vec3& lightPos);


// SSAO相关函数
void SSAOInit();


// 新增多重采样 FBO 句柄
unsigned int msFBO = 0;
unsigned int msColorRBO = 0;
unsigned int msDepthRBO = 0;



// 后处理四边形顶点数组对象和顶点缓冲对象
void FrameQuadInit(unsigned int& quadVAO, unsigned int& quadVBO);

// 尘埃星环
void RingGenerate(float outerRadius, float thickness);

int main()
{
    phys_test::Run();
    //warp_test::Run();
    warp_sc_test::Run();

    glfwInit(); // 初始化GLFW库
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // 设置OpenGL版本：主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // 设置OpenGL版本：次版本号
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 使用核心模式
    glfwWindowHint(GLFW_SAMPLES, 4);

    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // 创建窗口对象
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hello OpenGL", NULL, NULL);

    // 获取实际窗口大小（因为在某些平台上，窗口的实际大小可能与请求的大小不同）
    glfwGetFramebufferSize(window, &windowwidth, &windowheight);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate(); // 清理窗口资源
        return -1;
    }
    glfwMakeContextCurrent(window); // 设置当前窗口为上下文
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // 设置窗口大小回调函数
    glfwSetCursorPosCallback(window, mouse_callback); // 设置鼠标移动回调函数
    glfwSetScrollCallback(window, scroll_callback); // 设置鼠标滚轮回调函数

    // 捕获鼠标（隐藏鼠标光标，并提供无限的鼠标移动）
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 初始化GLAD，管理OpenGL函数指针，加载所有OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 纹理y轴翻转(因为OpenGL的y轴坐标是从下往上，而图片的y轴坐标是从上往下)
    stbi_set_flip_vertically_on_load(true);

    //加载深度缓冲
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE); // 启用多重采样抗锯齿
    setupFramebuffers(windowwidth, windowheight); // 设置离屏渲染帧缓冲

    // 创建着色器对象
   /* Shader planetshader("res/shader/00_SpaceShip/instancingVER.shader",
        "res/shader/00_SpaceShip/instancingFRAG3.0.shader");
    Shader asteroidShader("res/shader/00_SpaceShip/aster_ver.shader",
        "res/shader/00_SpaceShip/aster_frag3.0.shader");*/

        // G-buffer
        //Shader gBufferPlanetShader("res/shader/00_SpaceShip/G_buffer/gBuffer_planet_ver.shader",
        //    "res/shader/00_SpaceShip/G_buffer/gBuffer_planet_frag.shader");
    Shader gBufferAsteroidShader("res/shader/00_SpaceShip/G_buffer/gBuffer_asteroid_ver.shader",
        "res/shader/00_SpaceShip/G_buffer/gBuffer_asteroid_frag.shader");

    // 延迟着色
    Shader deferredLightingShader("res/shader/00_SpaceShip/Deferred_Shading2.0/deferred_lighting_ver.shader",
        "res/shader/00_SpaceShip/Deferred_Shading2.0/defer_light_ssao_frag2.0.shader");

    // 飞船 Spaceship
    Shader spaceshipShader("res/shader/00_SpaceShip/Forward_Shading/WarpSC/spaceship_ver4.0.shader",
        "res/shader/00_SpaceShip/Forward_Shading/WarpSC/spaceship_frag4.0.shader");


    // 火星
    Shader MarsShader("res/shader/00_SpaceShip/Forward_Shading/planet_ver.shader",
        "res/shader/00_SpaceShip/Forward_Shading/planet_frag2.0.shader");

    // 折跃：全屏闪现（阶段 0 占位为整屏常量白）
    Shader warpFlashShader("res/shader/00_SpaceShip/Warp/warp_flash_ver.shader",
        "res/shader/00_SpaceShip/Warp/warp_flash_frag.shader");
    // 折跃：电流外壳（同一套 mesh 沿法线外扩一点，加法混合再画一遍）
    Shader warpShellShader("res/shader/00_SpaceShip/WarpSC/shell/warp_shell_ver.shader",
        "res/shader/00_SpaceShip/WarpSC/shell/warp_shell_frag2.0.shader");
    // 折跃：线框（几何着色器打重心坐标，片元里只留三条边）
    Shader warpWireShader("res/shader/00_SpaceShip/WarpSC/wire/warp_wire_ver.shader",
        "res/shader/00_SpaceShip/WarpSC/wire/warp_wire_frag2.0.shader",
        "res/shader/00_SpaceShip/WarpSC/wire/warp_wire_geo.shader");
    // 折跃：幽灵舰体
    Shader warpGhostShader("res/shader/00_SpaceShip/WarpSC/ghost/warp_ghost_ver.shader",
        "res/shader/00_SpaceShip/WarpSC/ghost/warp_ghost_frag2.0.shader");
    // 折跃：起飞段碎屑（同一份立方体实例化）
    Shader warpDebrisShader("res/shader/00_SpaceShip/WarpSC/debris/warp_debris_ver.shader",
        "res/shader/00_SpaceShip/WarpSC/debris/warp_debris_frag.shader");
    // 折跃：冲击波（顶点着色器是自己的，片元复用碎屑那份 —— 四个 varying 语义相同）
    Shader warpShockShader("res/shader/00_SpaceShip/WarpSC/shock/warp_shock_ver.shader",
        "res/shader/00_SpaceShip/WarpSC/shock/warp_shock_frag.shader");
    // 折跃：冲击波的屏幕空间扭曲（全屏 pass，吃 quadVAO）
    Shader warpShockDistortShader("res/shader/00_SpaceShip/WarpSC/shock/warp_shock_distort_ver.shader",
        "res/shader/00_SpaceShip/WarpSC/shock/warp_shock_distort_frag2.0.shader");

    // IBL
    Shader equirectangularToCubemapShader("res/shader/#PBR/IBL3.0/cubemap_ver3.0.shader",
        "res/shader/#PBR/IBL3.0/cubemap_frag3.0.shader");
    Shader irradianceShader("res/shader/#PBR/IBL3.0/cubemap_ver3.0.shader",
        "res/shader/#PBR/IBL3.0/irradiance_frag.shader");

    Shader prefilterShader("res/shader/#PBR/IBL3.0/cubemap_ver3.0.shader",
        "res/shader/#PBR/IBL3.0/prefilter_frag.shader");
    Shader brdfShader("res/shader/#PBR/IBL3.0/BRDF_ver.shader",
        "res/shader/#PBR/IBL3.0/BRDF_frag.shader");

    // SSAO
    Shader ssao("res/shader/00_SpaceShip/SSAO/defer_light_ver.shader",
        "res/shader/00_SpaceShip/SSAO/ssao_frag.shader");
    Shader ssaoBlurShader("res/shader/00_SpaceShip/SSAO/defer_light_ver.shader",
        "res/shader/00_SpaceShip/SSAO/ssao_Blur_frag.shader");

    // Sun
    Shader sunCoreShader("res/shader/StarShader/StarList2.0/core_ver.shader",
        "res/shader/StarShader/StarList2.0/core_frag2.0.shader");
    Shader CoreCoronaShader("res/shader/StarShader/StarList2.0/corona_ver.shader",
        "res/shader/StarShader/StarList2.0/corona_frag2.0.shader");
    Shader sunCoronaShader("res/shader/StarShader/StarList2.0/corona_quad_ver.shader",
        "res/shader/StarShader/StarList2.0/corona_quad_frag2.0.shader");
    Shader sunGlowShader("res/shader/StarShader/StarList/star_glow_ver.shader",
        "res/shader/StarShader/StarList/star_glow_frag.shader");
    Shader sunVolShader("res/shader/00_SpaceShip/Stellar_Volumetric/sun_vol_ver.shader",
        "res/shader/00_SpaceShip/Stellar_Volumetric/sun_vol_2DN_frag.shader");

    // LensFlare
    Shader lensFlareShader("res/shader/LensFlareShader/lens_flare_ver.shader",
        "res/shader/LensFlareShader/lens_flare_frag.shader");


    // Skybox
    Shader spaceboxShader("res/shader/SkyBoxShader/SkyBox_ver.shader",
        "res/shader/SkyBoxShader/SkyBox_frag.shader");

    // Bloom
    Shader brightPassShader("res/shader/BloomShaders/bright_pass_ver.shader",
        "res/shader/BloomShaders/bright_pass_frag.shader");
    Shader blurShader("res/shader/BloomShaders/blur_ver.shader",
        "res/shader/BloomShaders/blur_frag.shader");
    Shader compositeShader("res/shader/BloomShaders/composite_ver.shader",
        "res/shader/BloomShaders/composite_frag2.0.shader");

    // Shadow
    Shader simpleDepthShader("res/shader/00_SpaceShip/depth_point3.0/depth_point_ver2.2.shader",
        "res/shader/00_SpaceShip/depth_point3.0/depth_point_frag.shader",
        "res/shader/00_SpaceShip/depth_point3.0/depth_point_geo.shader");

    // Stellar Ring
    Shader ringShader("res/shader/00_SpaceShip/Stellar_Ring2.0/ring_ver.shader",
        "res/shader/00_SpaceShip/Stellar_Ring2.0/ring_frag2.0.shader");

    // 行星大气散射
    Shader atmoShader("res/shader/00_SpaceShip/Atmosphere/atmo_ver.shader",
        "res/shader/00_SpaceShip/Atmosphere/atmo_frag.shader");


    // 小行星模型
    Model rock("res/model/rock/rock.obj");

    // 火星模型
    PbrModel planet("res/model/glb_model/planet/mars_2k.glb");

    // 飞船模型
    PbrModel spaceship("res/model/glb_model/homeworld_-_vaygr_battlecruiser_1k.glb");


    // 火星半径
    float planetRadius = 0.0f;
    for (auto& m : planet.meshes)
        for (auto& v : m.vertices)
            planetRadius = std::max(planetRadius, glm::length(v.Position));
    planetRadius *= 0.8f;   // planetScale = 0.8 均匀缩放

    //// 飞船包围半径
    //float shipBoundR = 0.0f;
    //for (auto& m : spaceship.meshes)
    //    for (auto& v : m.vertices)
    //        shipBoundR = std::max(shipBoundR, glm::length(v.Position));
    //shipBoundR *= 0.0005f;      // Spaceship::GetModelMatrix() 里的缩放

    // 飞船包围半径，以及轴向极值（计划书 §3.5 的溶解前沿要用）
    float shipBoundR = 0.0f;
    float shipAxisMinY = 1e30f;    // 局部 Y 的最小值：机头在这一端
    float shipAxisMaxY = -1e30f;
    for (auto& m : spaceship.meshes)
        for (auto& v : m.vertices)
        {
            shipBoundR = std::max(shipBoundR, glm::length(v.Position));
            shipAxisMinY = std::min(shipAxisMinY, v.Position.y);
            shipAxisMaxY = std::max(shipAxisMaxY, v.Position.y);
        }
    shipBoundR *= 0.0005f;      // Spaceship::GetModelMatrix() 里的缩放

    // ⚠ 注意：shipAxisMinY / shipAxisMaxY 【不乘 0.0005】。
    // 它们要在着色器里与 aPos 同空间比较，而 aPos 是原始局部坐标。乘了的话
    // 前沿会缩到 1/2000，整艘舰永远停在"全剥完"状态。

    ////// 临时探针：量舰体轴向范围与原点偏心（跑一次即可，看完删掉或注释保留）
    //std::printf("[hull] axisY obj [%.1f, %.1f]  world [%.4f, %.4f]  bowR=%.4f  len=%.4f\n",
    //    shipAxisMinY, shipAxisMaxY,
    //    shipAxisMinY * 0.0005f, shipAxisMaxY * 0.0005f,
    //    shipBoundR,
    //    (shipAxisMaxY - shipAxisMinY) * 0.0005f);
    //std::printf("[hull] origin_from_bow = %.3f of length  (0.50 = centered / 0.75 = origin is 1/4 from stern)\n",
    //    -shipAxisMinY / (shipAxisMaxY - shipAxisMinY));

    spaceshipShader.use();
    spaceshipShader.setInt("irradianceMap", 0);
    spaceshipShader.setInt("prefilterMap", 1);
    spaceshipShader.setInt("brdfLUT", 2);
    spaceshipShader.setInt("albedoMap", 3);
    spaceshipShader.setInt("normalMap", 4);
    spaceshipShader.setInt("metallicMap", 5);
    spaceshipShader.setInt("roughnessMap", 6);
    spaceshipShader.setInt("aoMap", 7);
    spaceshipShader.setInt("emissionMap", 8);
    spaceshipShader.setInt("depthMap", 9);   // 阴影 cubemap 用 unit9，避开 IBL/材质
    spaceshipShader.setInt("depthDynMap", 10);

    MarsShader.use();
    MarsShader.setInt("irradianceMap", 0);
    MarsShader.setInt("prefilterMap", 1);
    MarsShader.setInt("brdfLUT", 2);
    MarsShader.setInt("albedoMap", 3);
    MarsShader.setInt("normalMap", 4);
    MarsShader.setInt("metallicMap", 5);
    MarsShader.setInt("roughnessMap", 6);
    MarsShader.setInt("aoMap", 7);
    MarsShader.setInt("emissionMap", 8);
    MarsShader.setInt("depthMap", 9);   // 阴影 cubemap 用 unit9，避开 IBL/材质
    MarsShader.setInt("depthDynMap", 10);
    // 着色器初始化（个人风格问题，我更喜欢在循环体中去写
    //brightPassShader.use();
    //brightPassShader.setInt("hdrImage", 0);
    //blurShader.use();
    //blurShader.setInt("image", 0);


    // 加载光晕贴图
    unsigned int flareTexture = loadTexture("res/texture/lens_flare/lens_white.jpg");
    unsigned int flareTexture1 = loadTexture("res/texture/lens_flare/glow light lens flare(1).png");


    std::vector<Sun::LensFlare> flareTextures = {
        {flareTexture,  glm::vec4(1.0f, 0.8f, 0.5f, 0.6f), 3.0f},
        {flareTexture1, glm::vec4(1.0f, 0.8f, 0.5f, 0.6f), 3.0f}
    };

    // 阴影贴图初始化
    DepthCubeMapInit();

    // SSAO初始化
    // 配置 SSAO shader 的纹理单元（只需设置一次）
    SSAOInit();
    ssao.use();
    ssao.setInt("gPosition", 0);
    ssao.setInt("gNormal", 1);
    ssao.setInt("texNoise", 2);
    ssaoBlurShader.use();
    ssaoBlurShader.setInt("ssaoInput", 0);

    // 生成一个大型的半随机模型变换矩阵列表
    // ------------------------------------------------------------------
    unsigned int amount = 50000;
    RocksModelMatricesInit(amount, rock);


    // =======================================================
    // 程序化生成恒星顶点数据
    // 球体顶点数据
    std::vector<float> starVertices;
    std::vector<unsigned int> starIndices;
    // 生成球体顶点数据（可以使用UV球体或其他方法）
    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359f;
    BallGenerate(starVertices, starIndices, X_SEGMENTS, Y_SEGMENTS, PI);


    // 恒星 VAO, VBO, EBO
    unsigned int starVAO, starVBO, starEBO;

    // 日冕公告板顶点数据
    unsigned int coronaQuadVAO, coronaQuadVBO;

    // 创建恒星对象
    Sun Sun(starVAO, starVBO, starEBO, coronaQuadVAO, coronaQuadVBO);


    // =============================================
    // 帧缓冲四边形
    unsigned int quadVAO, quadVBO;
    FrameQuadInit(quadVAO, quadVBO);

    // =============================================
    // 天空盒顶点数据绑定
    unsigned int skyboxVAO, skyboxVBO;


    // 天空盒改用 IBL 生成的 envCubemap（SpaceBox 在 IBL 生成之后创建）


    // 尘埃星环
    RingGenerate(ringOuter, ringThickness);


    // PBR流程
    stbi_set_flip_vertically_on_load(true); // 
    int width, height, nrComponents;
    //float* data = stbi_loadf("res/texture/hdr/newport_loft.hdr", &width, &height, &nrComponents, 0);
    float* data = stbi_loadf("res/texture/hdr/space_fox.hdr", &width, &height, &nrComponents, 0);   // 获取HDR贴图信息
    // PBR初始化
    InitPBR(data, width, height, equirectangularToCubemapShader, irradianceShader, prefilterShader, brdfShader);



    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 天空盒改用 IBL 生成的 envCubemap（天空与光照环境统一）
    Skybox SpaceBox(skyboxVAO, skyboxVBO, envCubemap);

    // then before rendering, configure the viewport to the original framebuffer's screen dimensions
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);



    // 设置恒星，星球位置和大小
    glm::vec3 pointSunPositions = glm::vec3(-50.0f, 50.0f, -600.0f);
    //glm::vec3 SunScale = glm::vec3(120.0f);
    glm::vec3 planetPosition = glm::vec3(0.0f, -3.0f, 0.0f);
    gPlanetPos = planetPosition;        // 发布给 processInput（折跃落点合法性）
    gPlanetRadius = planetRadius;
    gShipBowOffset = shipBoundR;   // 包围球由 Y 向极值取到，所以它正好是舰首的偏移
    glm::vec3 planetScale = glm::vec3(0.8f);
    // 飞船位置
    //Spaceship ship;



    // ===== 定步长累加器 =====
    float physAccum = 0.0f;
    //constexpr float MAX_ACCUM = 0.25f;

    // ===== 折跃：起飞段碎屑的网格与实例缓冲（实现在 WarpDebris.cpp）=====
    WarpDebrisInit();


    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        // 计算帧时间
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // 输入管理
        processInput(window);

        // ---- 固定步长物理：把真实帧时间切成若干个 FIXED_DT ----
        float alpha = 0.0f;
        const int stepCount = ship.AdvanceFixed(physAccum, deltaTime, alpha);
        (void)stepCount;                                  // DEBUG_PHYS 关闭时避免"未使用"警告

        // ===== 折跃：时间线推进与传送 =====
        //  【现行】这里的 DT_MAX 与 advance 都在 WarpSC.h（24.x 走 SC 分支），不在 Warp.h。
        //  以下那行是 24.x 由 23.x 顺延时带过来的旧措辞（23.x 确实用 Warp.h，所以那两份是对的），
        //  留档对比：
        // 计时用真实 deltaTime，钳制在 Warp.h 内部做（1/30 s，计划书 §5.4）。
        // advance() 会在跨越 TAU_STAR 的那一帧把 tau 精确钉在峰值上并返回
        // teleport = true —— 传送必须且只能在这一帧执行（计划书 §4.4 命题 3）。
        if (gWarp.cooldown > 0.0f)
        {
            gWarp.cooldown -= deltaTime;
            if (gWarp.cooldown < 0.0f) gWarp.cooldown = 0.0f;
        }

        if (gWarp.active)
        {
            const warp_sc::Step wstep = warp_sc::advance(gWarp.tau, deltaTime);

            if (wstep.teleport)
            {
                // 舰体位移 delta。相机要不要跟着位移只看模式：只有跟随模式的
                // 相机锚在舰身上（下面那个相机块自身就被 if (currentMode ==
                // MODE_FOLLOW) 包着），另外两种模式的相机是玩家自己的
                // （MODE_REMOTE 固定、MODE_FREE 由 WASD 驱动），推它 100 单位
                // 是纯粹的错。
                //ship.WarpTo(ship.position + gWarp.delta);

                //if (currentMode == MODE_FOLLOW)
                //{
                //    camera.Position += gWarp.delta;
                //    // 阻尼 mix 不跳过：目标锚点 K* 同时也移动了同一个 delta，
                //    // 误差 e = K* - K 不变，递推原样继续（计划书 §5.2）。
                //}

                // 实际位移向量 = 冻结落点 - 当前位姿。舰体与相机必须用【同一个】向量，
                // 命题 2（舰体屏幕姿态不变、只有背景在跳）才继续精确成立。
                const glm::vec3 jump = gWarp.target - ship.position;
                ship.WarpTo(gWarp.target);

                if (currentMode == MODE_FOLLOW)
                {
                    camera.Position += jump;
                    // 阻尼 mix 不跳过：目标锚点 K* 同时也移动了同一个 jump，
                    // 误差 e = K* - K 不变，递推原样继续（计划书 §5.2）。
                }

                gWarp.cooldown = WARP_COOLDOWN;   // 冷却自 TAU_STAR 起算
            }

            if (wstep.finished)
            {
                gWarp.active = false;
                gWarp.tau = 0.0f;
            }
        }

        // 五个通道的权重。非活跃期 tau = 0，除 solid 外全为 0，就是常规舰体。
        const warp_sc::Channels wch = warp_sc::sample(gWarp.tau);
        // 实体度还要乘上"船身还在的比例"。真实溶解是逐像素 discard，u_v 与 u_w
        // 都不知道前沿在哪；不乘这一项会出现"影子已经是一整艘船、舰体还看不见"
        // （计划书 §5.3 的抵达侧那一例）。
        const float solidNow = wch.solid * (1.0f - warp_sc::geoFront(gWarp.tau));

        // 三种"相机侧"效果的共同门控：只有跟随模式的相机才随舰体平移。
        // 闪白要遮的背景视差跳变、FOV 冲击与抖动要卖的位移感，都只在这个模式下成立；
        // 模式 1/2 的相机是玩家自己的（固定 / 由 WASD 驱动），动它就是错的。
        const float camGate = (currentMode == MODE_FOLLOW) ? 1.0f : 0.0f;

        glm::mat4 shipModel = ship.GetModelMatrix(alpha);
        // 线框用的"真实姿态"：不含预备退距、不含拉伸。折跃时线框是蓝图，
        // 蓝图必须是舰体的最终形状，拉长的骨架读起来像另一艘船。
        const glm::mat4 shipWireModel = shipModel;

        // 沿航向的视觉位移 = 前向行程 − 预备退距。两个量都只进模型矩阵，
        // 前乘的平移叠加，所以合成一个 translate 更清楚。
        //// 前向行程（SC 分支）：去程冲出半个舰长、抵达从落点前滑回原位。
        //const float slideDist = gShipBowOffset * warp_sc::slideAmount(gWarp.tau);

        // 前向行程（SC 分支）：去程冲出半个舰长；抵达从骨架之外滑回原位，
        // 行程放大 SLIDE_ARRIVAL_K 倍，好让材质是"从外面飞进来"而不是"在原地变大"。
        const float slideDist = gShipBowOffset * warp_sc::slideFactor(gWarp.tau);
        const float backDist = warp_sc::KAPPA_BACK * WARP_DISTANCE * warp_sc::anticipation(gWarp.tau);
        const float alongFwd = slideDist - backDist;
        if (alongFwd != 0.0f)
        {
            shipModel = glm::translate(glm::mat4(1.0f), ship.RenderForward(alpha) * alongFwd) * shipModel;
        }


        // 阶段 3：沿局部 -Y 的拉伸，取代阶段 0 的整体缩放占位。

        // 拉伸：去程由 0.50 升到 τ*、保持线框段、与实体化同期收回（见 WarpSC.h）。
        // 锚点决定"哪一端先动"：+Y 是艉，锚在 MaxY 时艉钉住、艏被拉出去（去程）；
        // 锚在 MinY 时艏停在最终位置、只有艉收回（抵达）。切换点 τ*：
        // 那一帧舰体被完全剥除、外壳通道已归零，而线框与蓝图用的是不含拉伸的
        // shipWireModel，与锚点无关 —— 三种绘制都不受这次切换影响。
        const float stretch = warp_sc::stretchFactor(warp_sc::stretchAmount(gWarp.tau));
        if (stretch > 1.0f)
        {
            const float yAnchor = (gWarp.tau <= warp_sc::TAU_STAR) ? shipAxisMaxY : shipAxisMinY;
            // SC 分支：横向挤压。纵向拉 s 倍的同时把横截面按 1/pinch 收，
            // 读作"被抽走"而不是"被拉长"。倍率放在 WarpSC.h 的 PINCH_K，和 E_STRETCH 一起调。
            const float pinch = 1.0f + (stretch - 1.0f) * warp_sc::PINCH_K;

            shipModel = shipModel
                * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, yAnchor, 0.0f))
                * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / pinch, stretch, 1.0f / pinch))
                * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -yAnchor, 0.0f));
        }

        //// 临时探针：确认拉伸与滑入真的在跑（折跃期间每 10 帧打一次，看完即删）
        //static int dbgStretch = 0;
        //if (gWarp.active && (++dbgStretch % 10) == 0)
        //    std::printf("[stretch] tau=%.3f  a=%.3f  s=%.3f  pinch=%.3f  slide=%.1f\n",
        //        gWarp.tau, warp_sc::stretchAmount(gWarp.tau), stretch,
        //        1.0f + (stretch - 1.0f) * 1.6f, gShipBowOffset * warp_sc::slideFactor(gWarp.tau));


        // 碎屑锚点：每帧喂一次舰体姿态，模块自己判断要不要重新记（见 WarpDebris.cpp）
        WarpDebrisAnchor(shipModel);


        // 模式3：相机跟随飞船（后方偏上，看向飞船）
        if (currentMode == MODE_FOLLOW) {
            //glm::vec3 fwd = ship.Forward();
            glm::vec3 fwd = ship.RenderForward(alpha);
            //glm::vec3 target = ship.position - fwd * followDistance + glm::vec3(0.0f, followHeight, 0.0f);
            glm::vec3 target = ship.RenderPosition(alpha) - fwd * followDistance + glm::vec3(0.0f, followHeight, 0.0f);

            float t = glm::clamp(followSmooth * deltaTime, 0.0f, 1.0f);
            camera.Position = glm::mix(camera.Position, target, t);    // 平滑逼近，不再瞬移

            // look at ship, plus orbit look-around offset
            //glm::vec3 toShip = glm::normalize(ship.position - camera.Position);
            glm::vec3 toShip = glm::normalize(ship.RenderPosition(alpha) - camera.Position);
            glm::quat orbitRot = glm::angleAxis(glm::radians(orbitYaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
                glm::angleAxis(glm::radians(orbitPitch), glm::vec3(1.0f, 0.0f, 0.0f));

            camera.Front = orbitRot * toShip;
            camera.Right = glm::normalize(glm::cross(camera.Front, glm::vec3(0.0f, 1.0f, 0.0f)));
            camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));

            // 把跟随相机算出来的朝向同步回 Orient —— 否则模式 3 里鼠标会往 Orient 上偷偷
            // 累积旋转（看不见），一切回模式 1/2，第一次鼠标事件就用它重算向量，镜头会跳。
            camera.SyncOrientFromVectors();
        }


        RockViewFrustumCull(window, pointSunPositions);

        // ====== 阴影 Pass ======
        glm::mat4 shadowProj = glm::perspective(
            glm::radians(90.0f),
            (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT,
            shadow_near, shadow_far);

        // 1) 6 面矩阵 + 绑 FBO + 只清一次（分层附件 → 一次清 6 面）
        std::vector<glm::mat4> shadowTransforms;
        ShadowPassBegin(depthCubeFBO, SHADOW_WIDTH, shadowProj, shadowTransforms, pointSunPositions);

        // 2) 公共 uniform
        simpleDepthShader.use();
        //simpleDepthShader.setInt("faceIndex", StaticShadowFace(pointSunPositions, planetPosition));
        for (unsigned int i = 0; i < 6; ++i)
            simpleDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        simpleDepthShader.setFloat("far_plane", shadow_far);
        simpleDepthShader.setVec3("lightPos", pointSunPositions);

        // 3) 逐面画小行星（空面跳过，每面只画分到它的那批
        simpleDepthShader.setBool("instanced", true);
        for (int f = 0; f < 6; ++f)
        {
            if (gFaceCount[f] == 0) continue;
            simpleDepthShader.setInt("faceIndex", f);

            glBindBuffer(GL_ARRAY_BUFFER, rockInstanceVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                gFaceCasters[f].size() * sizeof(glm::mat4), gFaceCasters[f].data());

            for (unsigned int i = 0; i < rock.meshes.size(); i++)
            {
                glBindVertexArray(rock.meshes[i].VAO);
                glDrawElementsInstanced(GL_TRIANGLES,
                    static_cast<unsigned int>(rock.meshes[i].indices.size()),
                    GL_UNSIGNED_INT, 0, gFaceCount[f]);
                glBindVertexArray(0);
            }
        }



        // 4) 行星 / 飞船：同样分面，只画进各自所在的面
        simpleDepthShader.setBool("instanced", false);

        glm::mat4 sdModel = glm::mat4(1.0f);
        sdModel = glm::translate(sdModel, planetPosition);
        sdModel = glm::scale(sdModel, planetScale);
        simpleDepthShader.setMat4("model", sdModel);

        // --- 行星 ---
        int pf[6];
        int pn = AssignCasterFaces(pointSunPositions, planetPosition, planetRadius, pf);
        for (int k = 0; k < pn; ++k)
        {
            simpleDepthShader.setInt("faceIndex", pf[k]);
            planet.Draw(simpleDepthShader);
        }


        // --- 飞船 ---
        // 折跃期间"舰体实体度"过半就停止投射：舰体已经看不见而影还留在星环上，
        // 一眼露馅（计划书 §5.3）。深度 pass 做不了半透明，所以是二值判据。
        if (solidNow > 0.5f)
        {
            //simpleDepthShader.setMat4("model", ship.GetModelMatrix());
            simpleDepthShader.setMat4("model", shipModel);

            //int sf[6]; int sn = AssignCasterFaces(pointSunPositions, ship.position, shipBoundR, sf);
            //int sf[6]; int sn = AssignCasterFaces(pointSunPositions, ship.RenderPosition(alpha), shipBoundR, sf);
            //  分面指派必须用【画出来的】那一份位置与半径：舰体的模型矩阵带着前冲（最多
            //  SLIDE_DEPART_K * gShipBowOffset）与拉伸，用逻辑位置指派会让影子在面的边界附近跳。
            //  alongFwd 就是模型矩阵实际被平移的量（前冲 − 预备退距），radius 乘上拉伸倍数，
            //  否则拉长后的舰体会超出包围球、某些面上缺一块。
            const glm::vec3 casterPos = ship.RenderPosition(alpha) + ship.RenderForward(alpha) * alongFwd;
            int sf[6]; int sn = AssignCasterFaces(pointSunPositions, casterPos,
                shipBoundR * std::max(1.0f, stretch), sf);
            for (int k = 0; k < sn; ++k)
            {
                simpleDepthShader.setInt("faceIndex", sf[k]);
                spaceship.Draw(simpleDepthShader);
            }
        }


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCullFace(GL_BACK);
        // ====== 阴影 Pass 结束 ======


        // ====== 几何pass开头
        glViewport(0, 0, windowwidth, windowheight);
        // 渲染
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);  // 绑定到HDR帧缓冲
        glViewport(0, 0, windowwidth, windowheight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        // 配置变换矩阵
        int winWidth, winHeight;
        glfwGetFramebufferSize(window, &winWidth, &winHeight);
        float aspect = winWidth / (float)winHeight;
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 2000.0f);

        //glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), aspect, 0.1f, 2000.0f);
        //glm::mat4 view = camera.GetViewMatrix();

                // FOV 冲击：闪现前后把视场角收紧 Φ，再放回来，读作一次推镜。
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Fov - warp_sc::FOV_KICK * (WARP_DISTANCE / 100.0f) * warp_sc::fovKick(gWarp.tau) * camGate),
            aspect, 0.1f, 2000.0f);

        // 短促抖动：沿 heading 的一个位移脉冲。**只在本帧的 view 上生效，用完立刻还原。**
        // 绝不能把它留在 camera.Position 里 —— 跟随阻尼下一帧会从"被抖过的位置"起步，
        // 抖动就被记住了，而且会污染距离不变式的验收。
        const glm::vec3 camShake =
            ship.RenderForward(alpha) * (warp_sc::SHAKE_AMT * (WARP_DISTANCE / 100.0f) * warp_sc::shakeEnvelope(gWarp.tau) * camGate);
        camera.Position += camShake;
        glm::mat4 view = camera.GetViewMatrix();
        camera.Position -= camShake;


        // ===== G-Buffer Pass =====

        // === 几何 Pass: Planet === (火星改前向渲染，不再进 G-Buffer)
        //gBufferPlanetShader.use();
        //gBufferPlanetShader.setMat4("projection", projection);
        //gBufferPlanetShader.setMat4("view", view);
        //glm::mat4 model = glm::mat4(1.0f);
        //model = glm::translate(model, planetPosition);
        //model = glm::scale(model, planetScale);
        //gBufferPlanetShader.setMat4("model", model);
        //planet.Draw(gBufferPlanetShader);

        // === G-Buffer Pass: Asteroids ===
        gBufferAsteroidShader.use();
        gBufferAsteroidShader.setMat4("projection", projection);
        gBufferAsteroidShader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rock.textures_loaded[0].id);
        gBufferAsteroidShader.setInt("material_diffuse", 0);
        if (!gVisible.empty())
        {
            glBindBuffer(GL_ARRAY_BUFFER, rockInstanceVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                gVisible.size() * sizeof(glm::mat4), gVisible.data());
        }
        for (unsigned int i = 0; i < rock.meshes.size(); i++)
        {
            glBindVertexArray(rock.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES,
                static_cast<unsigned int>(rock.meshes[i].indices.size()),
                GL_UNSIGNED_INT, 0, rockVisibleCount);
            glBindVertexArray(0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // === 将 G-Buffer 深度传到 hdrFBO ===
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO);
        glBlitFramebuffer(0, 0, windowwidth, windowheight,
            0, 0, windowwidth, windowheight,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST);


        // === SSAO Pass ===
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        ssao.use();
        for (unsigned int i = 0; i < 64; ++i)
            ssao.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
        ssao.setMat4("projection", projection);
        ssao.setMat4("view", view);
        ssao.setBool("unify", unify);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // === SSAO Blur Pass ===
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        ssaoBlurShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // === 光照 Pass ===
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        deferredLightingShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        deferredLightingShader.setInt("gPosition", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        deferredLightingShader.setInt("gNormal", 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        deferredLightingShader.setInt("gAlbedo", 2);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gPBR);
        deferredLightingShader.setInt("gPBR", 3);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
        deferredLightingShader.setInt("depthMap", 4);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
        deferredLightingShader.setInt("ssao", 5);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        deferredLightingShader.setInt("irradianceMap", 6);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        deferredLightingShader.setInt("prefilterMap", 7);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
        deferredLightingShader.setInt("brdfLUT", 8);
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthDynMap);
        deferredLightingShader.setInt("depthDynMap", 10);


        deferredLightingShader.setVec3("lightPos", pointSunPositions);
        deferredLightingShader.setVec3("viewPos", camera.Position);
        deferredLightingShader.setFloat("far_plane", shadow_far);
        deferredLightingShader.setBool("shadows", shadows);
        deferredLightingShader.setBool("PCSS", PCSS);
        deferredLightingShader.setBool("ssaoEnabled", ssaoEnabled);

        // 菲涅尔边缘光（延迟着色 — 小行星等）
        deferredLightingShader.setVec3("rimColor", glm::vec3(0.6f, 0.55f, 0.5f));  // 暖灰，模拟阳光掠射
        deferredLightingShader.setFloat("rimPower", 3.0f);
        deferredLightingShader.setFloat("rimStrength", 0.35f);

        // PointLight[0]
        deferredLightingShader.setVec3("pointLights[0].position", pointSunPositions);
        deferredLightingShader.setVec3("pointLights[0].color", glm::vec3(200.0f, 200.0f, 160.0f));
        //deferredLightingShader.setVec3("pointLights[0].ambient", 1.0f, 1.0f, 0.8f);
        deferredLightingShader.setFloat("pointLights[0].constant", 1.0f);
        deferredLightingShader.setFloat("pointLights[0].linear", 0.0002f);
        deferredLightingShader.setFloat("pointLights[0].quadratic", 0.000005f);

        deferredLightingShader.setFloat("shininess", 32.0f);

        glDepthMask(GL_FALSE);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDepthMask(GL_TRUE);


        // =================================
        // ===== 天空盒（背景）=====
        // =================================
        SpaceBox.SkyboxRender(spaceboxShader, camera, projection);
        // ======= 天空盒绘制结束 ========


        // 先保存当前的深度状态和混合状态，以便后续恢复
        GLboolean depthEnabled;
        glGetBooleanv(GL_DEPTH_TEST, &depthEnabled);
        GLboolean blendEnabled;
        glGetBooleanv(GL_BLEND, &blendEnabled);
        GLint depthFunc;
        glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
        GLboolean depthMask;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);



        // ========================================
        // 恒星渲染部分 - 开始
        // ========================================
        //float starTime = static_cast<float>(glfwGetTime()); // 恒星旋转时间
        //float starPulse = 1.0f + sin(starTime * 1.5f) * 0.0003f; // 计算脉冲效果

        Sun.SunRender(sunCoreShader, sunCoronaShader, CoreCoronaShader, sunGlowShader, camera, projection, view);
        //Sun.SunRenderPlus(sunCoreShader, sunCoronaShader, CoreCoronaShader, sunGlowShader, sunVolShader, camera, projection, view);

        // ========================================
        // 恒星渲染部分 - 结束
        // ========================================



        // 恢复原始状态
        if (!depthEnabled) glDisable(GL_DEPTH_TEST);
        else glEnable(GL_DEPTH_TEST);
        glDepthMask(depthMask ? GL_TRUE : GL_FALSE);
        glDepthFunc(depthFunc);
        if (blendEnabled) glEnable(GL_BLEND);
        else glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 恢复默认混合函数


        // ====== 火星 forward PBR 渲染 ======
        glm::mat4 planetModel = glm::mat4(1.0f);
        planetModel = glm::translate(planetModel, planetPosition);
        planetModel = glm::scale(planetModel, planetScale);

        MarsShader.use();
        MarsShader.setMat4("projection", projection);
        MarsShader.setMat4("view", view);
        MarsShader.setMat4("model", planetModel);
        MarsShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(planetModel))));
        MarsShader.setVec3("camPos", camera.Position);

        MarsShader.setVec3("lightPositions[0]", pointSunPositions);
        MarsShader.setVec3("lightColors[0]", glm::vec3(200.0f, 200.0f, 160.0f));
        MarsShader.setFloat("lightConstant", 1.0f);
        MarsShader.setFloat("lightLinear", 0.0002f);
        MarsShader.setFloat("lightQuadratic", 0.000005f);

        // 星球材质：非金属、高粗糙
        MarsShader.setBool("useMetallicMap", true);
        MarsShader.setBool("useRoughnessMap", true);
        MarsShader.setFloat("metallicValue", 0.0f);
        MarsShader.setFloat("roughnessValue", 1.0f);
        MarsShader.setBool("useEmissiveMap", false);
        MarsShader.setBool("useAOMap", false);
        MarsShader.setFloat("aoValue", 1.0f);
        MarsShader.setBool("shadows", true);
        MarsShader.setBool("PCSS", false);
        MarsShader.setFloat("far_plane", shadow_far);

        // 菲涅尔边缘光：星球淡蓝大气轮廓
        MarsShader.setVec3("rimColor", glm::vec3(0.4f, 0.7f, 1.0f));
        MarsShader.setFloat("rimPower", 4.0f);
        MarsShader.setFloat("rimStrength", 0.15f);

        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
        glActiveTexture(GL_TEXTURE9); glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
        glActiveTexture(GL_TEXTURE10); glBindTexture(GL_TEXTURE_CUBE_MAP, depthDynMap);

        glCullFace(GL_BACK);
        planet.Draw(MarsShader);


        // ====== 行星大气散射（独立大气壳，背光透光晕）======
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glCullFace(GL_BACK);

        atmoShader.use();
        atmoShader.setVec3("camPos", camera.Position);
        atmoShader.setVec3("planetCenter", planetPosition);
        atmoShader.setFloat("planetRadius", planetRadius);
        atmoShader.setFloat("atmoScale", 1.25f);                    // 大气壳半径倍数
        glm::vec3 sunDirNorm = glm::normalize(pointSunPositions - planetPosition);
        atmoShader.setVec3("sunDir", sunDirNorm);                  // 从行星指向太阳
        atmoShader.setVec3("sunColor", glm::vec3(1.0f, 0.9f, 0.75f));
        atmoShader.setFloat("density", 1.0f);
        atmoShader.setFloat("intensity", 0.15f);
        //atmoShader.setVec3("rayleighCoef", glm::vec3(0.005f, 0.008f, 0.02f));  // 蓝偏
        atmoShader.setVec3("rayleighCoef", glm::vec3(0.05f, 0.08f, 0.2f));  // ×100 测试，原来 0.005/0.008/0.02
        atmoShader.setFloat("miecoef", 0.02f);
        atmoShader.setVec2("resolution", glm::vec2((float)windowwidth, (float)windowheight));
        atmoShader.setMat4("invProjView", glm::inverse(projection * view));

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);


        // ====== 尘埃星环：全屏 ray march 体积 ======
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   // 原 GL_SRC_ALPHA, GL_ONE
        glDisable(GL_DEPTH_TEST);      // 关键：让 shader 自己用 ray-sphere 做火星遮挡
        glDepthMask(GL_FALSE);
        glCullFace(GL_BACK);

        ringShader.use();
        ringShader.setVec3("camPos", camera.Position);
        ringShader.setVec3("ringCenter", planetPosition);
        ringShader.setFloat("ringInner", ringInner);
        ringShader.setFloat("ringOuter", ringOuter);
        ringShader.setFloat("ringHalfHeight", ringThickness * 0.5f);
        ringShader.setFloat("planetRadius", planetRadius);
        ringShader.setVec3("ringColor", glm::vec3(0.8f, 0.6f, 0.4f));
        ringShader.setFloat("time", static_cast<float>(glfwGetTime()));
        ringShader.setVec3("sunPos", pointSunPositions);
        ringShader.setVec3("sunColor", glm::vec3(1.0f, 0.85f, 0.6f));
        ringShader.setFloat("far_plane", shadow_far);

        ringShader.setVec2("resolution", glm::vec2((float)windowwidth, (float)windowheight));
        ringShader.setMat4("invProjView", glm::inverse(projection * view));

        ringShader.setInt("depthMap", 9);
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);

        ringShader.setInt("depthDynMap", 10);
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthDynMap);

        glBindVertexArray(quadVAO);          // 用全屏 quad，不再用 ringVAO
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);             // 恢复
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);



        // ====== 飞船 forward PBR 渲染 ======
        glDepthMask(GL_TRUE);

        //glm::mat4 spaceshipModel = ship.GetModelMatrix();
        glm::mat4 spaceshipModel = shipModel;
        spaceshipShader.use();
        spaceshipShader.setMat4("projection", projection);
        spaceshipShader.setMat4("view", view);
        spaceshipShader.setMat4("model", spaceshipModel);
        spaceshipShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(spaceshipModel))));
        spaceshipShader.setVec3("camPos", camera.Position);

        spaceshipShader.setVec3("lightPositions[0]", pointSunPositions);
        spaceshipShader.setVec3("lightColors[0]", glm::vec3(200.0f, 200.0f, 160.0f));
        spaceshipShader.setFloat("lightConstant", 1.0f);
        spaceshipShader.setFloat("lightLinear", 0.0002f);
        spaceshipShader.setFloat("lightQuadratic", 0.000005f);

        spaceshipShader.setBool("useMetallicMap", true);
        spaceshipShader.setBool("useRoughnessMap", true);
        spaceshipShader.setFloat("metallicValue", 0.0f);
        spaceshipShader.setFloat("roughnessValue", 0.5f);
        spaceshipShader.setBool("useEmissiveMap", true);
        spaceshipShader.setBool("useAOMap", false);
        spaceshipShader.setFloat("aoValue", 1.0f);
        spaceshipShader.setFloat("emissiveStrength", 2.0f);
        // 菲涅尔
        spaceshipShader.setVec3("rimColor", glm::vec3(0.4f, 0.6f, 1.0f));  // 淡蓝 ( > 1.0 时会参与到Bloom)
        spaceshipShader.setFloat("rimPower", 3.0f);                        // 越大边缘越锐利
        //spaceshipShader.setFloat("rimStrength", 0.6f);                     // 强度 

        // 折跃占位：电流通道 u_c 放大现有边缘光，先不新增绘制（计划书 §8 阶段 0）
        spaceshipShader.setFloat("rimStrength", warp_tune::RIM_BASE * (1.0f + warp_tune::RIM_CHARGE_GAIN * wch.c));
        // 折跃：两级轴向溶解（计划书 §3.5）
        spaceshipShader.setFloat("uVanish", wch.v);
        spaceshipShader.setFloat("uWhiteK", warp_tune::U_WHITE_K);


        const float frontGeo = warp_sc::geoFront(gWarp.tau);
        // 坐标镜像只在实体化段打开：去程艏先没（不需要镜像），
        // 抵达要让材质自艏向艉恢复（需要镜像）。推导写在 4.0 片元着色器里。
        spaceshipShader.setFloat("uFrontRev", (gWarp.tau > warp_sc::T_WIRE_END) ? 1.0f : 0.0f);
        spaceshipShader.setFloat("uFrontMat", warp_sc::matFront(gWarp.tau));
        spaceshipShader.setFloat("uFrontGeo", frontGeo);
        spaceshipShader.setFloat("uTranslucency", warp_tune::U_TRANSLUCENCY);   // 半透明平台：丢弃六成格子、保留四成
        spaceshipShader.setFloat("uFrontSoft", warp_tune::U_FRONT_SOFT);      // 半透明波前沿的过渡宽度，必须大于 0
        spaceshipShader.setFloat("uSolidify", warp_sc::solidify(gWarp.tau));   // 抵达段整体凝实：0 → 1

        spaceshipShader.setFloat("uAxisMinY", shipAxisMinY);
        spaceshipShader.setFloat("uAxisMaxY", shipAxisMaxY);
        spaceshipShader.setFloat("uObjRadius", shipBoundR / 0.0005f);
        spaceshipShader.setFloat("uNoiseW", warp_sc::NOISE_W);
        spaceshipShader.setFloat("uNoiseFreq", warp_tune::U_NOISE_FREQ);
        spaceshipShader.setFloat("uEdge", warp_tune::U_EDGE);
        spaceshipShader.setVec3("uDissolveColor", warp_tune::U_DISSOLVE_COLOR);

        spaceshipShader.setBool("shadows", true);
        spaceshipShader.setBool("PCSS", false);
        spaceshipShader.setFloat("far_plane", shadow_far);

        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
        glActiveTexture(GL_TEXTURE9); glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
        glActiveTexture(GL_TEXTURE10); glBindTexture(GL_TEXTURE_CUBE_MAP, depthDynMap);

        glCullFace(GL_BACK);
        spaceship.Draw(spaceshipShader);


        // ===== 折跃：电流外壳 =====
        // 门控与强度都用外壳自己的包络 shellAlpha（见 WarpSC.h）：它不再挂 u_c，
        // 于是"外壳什么时候开始暗、什么时候归零"可以独立于互相咬合的通道表来调。
        // PbrMesh::Draw 只会顺手设几个材质贴图 uniform（本程序里不存在，
        // glUniform1i(-1, ...) 是空操作），所以可以安全地复用同一套 mesh。
        //if (wch.c > 0.001f)
        if (warp_sc::shellAlpha(gWarp.tau) > 0.001f)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);      // 加法混合
            glDepthMask(GL_FALSE);            // 不写深度，别挡住后面要画的东西
            glCullFace(GL_FRONT);             // 只画背面：读起来才是"裹住"而不是"镀一层"

            warpShellShader.use();
            warpShellShader.setMat4("projection", projection);
            warpShellShader.setMat4("view", view);
            //  外壳在扫描开始时就归零（见 shellAlpha），所以它永远活不到 τ*，
            //  可以放心跟 spaceshipModel（含前冲与拉伸，贴在舰体表面上）。
            //  注意：若以后又把它的寿命拉过 τ*，就必须改用不含滑入的那一份矩阵，
            //  否则会被滑入的 −SLIDE_ARRIVAL_K 个半舰长甩在起点那一侧。
            warpShellShader.setMat4("model", spaceshipModel);   // 与 PBR pass 同一份

            warpShellShader.setVec3("camPos", camera.Position);
            warpShellShader.setVec3("uShellColor", warp_tune::U_SHELL_COLOR);
            warpShellShader.setFloat("uShellStrength", warp_sc::shellAlpha(gWarp.tau)
                * warp_tune::U_SHELL_GAIN);
            warpShellShader.setFloat("uShellWorld", shipBoundR * warp_tune::U_SHELL_WORLD_K);
            warpShellShader.setFloat("uObjRadius", shipBoundR / 0.0005f);
            warpShellShader.setFloat("uTime", currentFrame);
            warpShellShader.setFloat("uGridFreq", warp_tune::U_GRID_FREQ);       // 格子密度：26.8 / 12 ≈ 2.2 世界单位一格
            warpShellShader.setFloat("uGridWidth", warp_tune::U_GRID_WIDTH);     // 格线宽度，别超过 0.2
            warpShellShader.setFloat("uGridMix", warp_tune::U_GRID_MIX);         // 1 = 两组叠加；调 0 就退回纯正方格
            warpShellShader.setFloat("uGridStrength", warp_tune::U_GRID_STRENGTH);
            spaceship.Draw(warpShellShader);

            glCullFace(GL_BACK);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        // ===== 折跃：目的地蓝图（SC 分支已删除）=====
        // SC2 的落点严格排在"舰体完全消失"之后，所以蓄能期不画任何落点几何。
        // 落点的第一件东西是线框骨架，它在 tau* 之后才自艉向艏长出（见 WarpSC.h 的 wireGrow）。

        // ===== 折跃：起飞段碎屑（实现在 WarpDebris.cpp）=====
        //WarpDebrisDraw(warpDebrisShader, projection, view, camera.Position, shipBoundR);

        // ===== 折跃：光柱（实现在 WarpDebris.cpp）=====
        // 锚点 = 带上"抵达滑入起点"偏移的逻辑位姿，也就是【带材质的舰船出现的那张面】：
        // 离落点 SLIDE_ARRIVAL_K 个半舰长（约 67.7）处。
        // 不带这个偏移就会落在舰体中垂面上，读起来就变成"幽灵舰体本身"。
        // 偏移只对抵达段生效，tau* 之前用逻辑位姿本身。
        /*const float pillarSlide =
            (gWarp.tau > warp_sc::TAU_STAR) ? (-warp_sc::SLIDE_ARRIVAL_K * gShipBowOffset) : 0.0f;*/
        const float pillarSlide =
            (gWarp.tau > warp_sc::TAU_STAR)
            ? (warp_sc::P_PLANE_SIDE * warp_sc::SLIDE_ARRIVAL_K * gShipBowOffset) : 0.0f;
        const glm::mat4 pillarPlane =
            glm::translate(glm::mat4(1.0f), ship.RenderForward(alpha) * pillarSlide) * shipWireModel;
        WarpPillarDraw(warpDebrisShader, projection, view, camera.Position, shipBoundR, pillarPlane);


        // ===== 折跃：冲击波（实现在 WarpDebris.cpp）=====
        //  两道波绕舰体纵轴、共用同一支绘制，区别只在锚点：
        //    去程收束波 → 逻辑位姿（波跟着船收拢）
        //    抵达发散波 → 光柱那张面（波从材质出生的地方散开）
        {
            const bool shockOut = warp_sc::shockOutLive(gWarp.tau);
            WarpShockDraw(warpShockShader, projection, view, camera.Position, shipBoundR,
                shockOut ? shipWireModel : pillarPlane);
        }

        // ===== 折跃：幽灵舰体（落点半透明舰体）=====
        // SC2 的抵达里，落点先出现一层半透明的舰体轮廓，随后材质才自艉向艏补上。
        // 它与线框骨架共用同一根生长前沿（同一个 uGrow 与同一套轴范围），两件东西
        // 因此同步自艉向艏长出 —— 注意这句的方向与材质相反：材质是自艏向艉填充的。
        // 强度挂 1 - solidify：整段骨架期满强度，材质填充开始时开始退，
        // 填充完成时正好退完（单调，不会重新冒出来）。
        // 加法混合且不写深度：加法满足交换律，所以它与线框、外壳、碎屑之间都不需要排序。
        // 本工程从不启用面剔除，正反两面都会加进来，正好读作"能看见内部结构"。
        if (wch.w > 0.001f && gWarp.tau >= warp_sc::TAU_STAR)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glDepthMask(GL_FALSE);

            warpGhostShader.use();
            warpGhostShader.setMat4("projection", projection);
            warpGhostShader.setMat4("view", view);
            warpGhostShader.setMat4("model", shipWireModel);   // 与线框同一份：落点的最终形状
            warpGhostShader.setVec3("camPos", camera.Position);
            warpGhostShader.setVec3("uGhostColor", warp_tune::U_GHOST_COLOR);

            // 用 1 - solidify 而不是 1 - wch.a：后者挂的是梯形通道，末段会自己升回来，
            // 让幽灵舰体在收尾时重新冒出来（此前被 wch.w 的旧门控挡住，延长骨架窗口后就会露出来）。
            warpGhostShader.setFloat("uGhostStrength", warp_sc::ghostAlpha(gWarp.tau) * warp_tune::U_GHOST_GAIN);
            warpGhostShader.setFloat("uGhostExpand", shipBoundR * warp_tune::U_GHOST_EXPAND_K);
            warpGhostShader.setFloat("uAxisMinY", shipAxisMinY);
            warpGhostShader.setFloat("uAxisMaxY", shipAxisMaxY);
            warpGhostShader.setFloat("uGrow", warp_sc::wireGrow(gWarp.tau));
            warpGhostShader.setFloat("uGrowSoft", warp_tune::U_GHOST_GROW_SOFT);
            warpGhostShader.setFloat("uRimPow", warp_tune::U_GHOST_RIM_POW);
            warpGhostShader.setFloat("uBody", warp_tune::U_GHOST_BODY);

            // 抵达段的多道扫描波（7 道、周期 = 1 舰长）：速度与去程扫描对齐，
            // 也就是"一整舰长用 SCAN_TIME 秒"，于是相位直接取 (tau - TAU_STAR) / SCAN_TIME。
            // tau = TAU_STAR 时相位为 0，第 1 道正好压在艏上（与参考画面那张表的起点一致）。
            // 【现行】波列速度与去程扫描脱钩：抵达段有 4.07 秒，用 SCAN_TIME（0.60）等于跑
            // 6.8 趟，且最紧那一对（0.05 舰长）只停留 0.030 秒，观感是"闪过去"。
            // 现按参考画面读数取"整段 1.75 个舰长"，即 2.3257 s/舰长。下面那两行是旧写法，留档。
            // 速度与去程扫描对齐，
            // 也就是"一整舰长用 SCAN_TIME 秒"，于是相位直接取 (tau - TAU_STAR) / SCAN_TIME。
            // tau = TAU_STAR 时相位为 0，第 1 道正好压在艏上（与参考画面那张表的起点一致）；
            // tau = T_TOTAL 时相位正好 1.75，即刚好走完你量的那个路程。
            warpGhostShader.setFloat("uBandPhase", (gWarp.tau - warp_sc::TAU_STAR) / warp_sc::BAND_SWEEP_TIME);
            //warpGhostShader.setFloat("uBandW", warp_tune::U_GHOST_BAND_W);   // 旧：世界空间宽度
            warpGhostShader.setFloat("uBandPx", warp_tune::U_GHOST_BAND_PX);
            warpGhostShader.setVec3("uBandStrong", warp_tune::U_GHOST_BAND_STRONG);
            warpGhostShader.setVec3("uBandWeak", warp_tune::U_GHOST_BAND_WEAK);
            warpGhostShader.setFloat("uBandGate", warp_sc::wireAlpha(gWarp.tau));

            spaceship.Draw(warpGhostShader);

            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }


        // ===== 折跃：线框 =====
        // 启用条件是"传送已经发生"（tau >= TAU_STAR），不是"u_w 非零"：
        // u_w 从 0.62 就起来了，而 tau* = 0.65 —— 只判 u_w 的话，那段里线框会
        // 画在**起点**（它用的是同一个 shipModel），违反"去程不产生线框"。
        if (wch.w > 0.001f && gWarp.tau >= warp_sc::TAU_STAR)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);        // 加法混合
            glDepthMask(GL_FALSE);              // 只读深度、不写深度
            //glDisable(GL_CULL_FACE);            // 两面都画：读起来才是"全息透视线框"

            warpWireShader.use();
            warpWireShader.setMat4("projection", projection);
            warpWireShader.setMat4("view", view);
            // 线框
            //warpWireShader.setMat4("model", spaceshipModel);   // 与 PBR pass 同一份

            // 线框故意用"不拉伸"的那一份：它是舰体的最终形状（蓝图），
            // 拉伸只属于被拽出去的实体。外壳 pass 仍用拉伸后的矩阵，
            // 因为它贴在舰体表面上，必须跟着形变走。
            warpWireShader.setMat4("model", shipWireModel);

            //warpWireShader.setVec3("uWireColor", warp_tune::U_WIRE_COLOR);
            warpWireShader.setVec3("uWireColor", warp_tune::U_WIRE_COLOR_BONE);
            //warpWireShader.setFloat("uWireAlpha", wch.w);
            warpWireShader.setFloat("uWireAlpha", warp_sc::wireAlpha(gWarp.tau));   // 与蓝图同一条包络
            warpWireShader.setFloat("uWireGrow", warp_sc::wireGrow(gWarp.tau));
            warpWireShader.setFloat("uWireWidth", warp_tune::U_WIRE_WIDTH);
            warpWireShader.setFloat("uAxisMinY", shipAxisMinY);
            warpWireShader.setFloat("uAxisMaxY", shipAxisMaxY);
            spaceship.Draw(warpWireShader);

            //glEnable(GL_CULL_FACE);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        // 拒绝反馈红闪：逐帧衰减，与折跃时间线无关。刻意不乘 camGate —— 它是给玩家的
        // 反馈，三种模式都要看见；模式 1/2 关掉的只是"遮背景"的那一下白闪。
        if (gWarp.reject > 0.0f) gWarp.reject -= deltaTime;
        const float rejectFlash = warp_sc::quintic01(gWarp.reject / WARP_REJECT_FLASH);

        // ===== 折跃：全屏闪现 =====
        // 它的唯一职责是遮住"背景视差跳变"，而背景跳不跳只看相机有没有跟着平移。
        //  MODE_FOLLOW：相机随舰体平移 100 单位，行星角位移可达数十度
        //               （计划书 §4.2 推论 2），必须整屏覆盖，否则画面边缘会
        //               露出背景的跳变。
        //  模式 1/2：相机不动，世界固定物的投影逐像素完全不变，没有跳变要遮；
        //            而且这两种视角恰恰是"想看清舰体折跃"的视角，一整屏白
        //            反而把要看的东西盖住了。所以直接关掉。
        // 模式 1/2 下舰体自己在屏幕上换位置那一下，由 u_v 白化负责（阶段 3）。
        const float flashAmount = wch.g * camGate;

        if (flashAmount > 0.0f || rejectFlash > 0.0f)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);        // 加法混合
            glDisable(GL_DEPTH_TEST);           // 全屏 quad 不需要深度
            glDepthMask(GL_FALSE);

            // 舰体的屏幕位置（NDC），作为 bloom 的圆心。直接投影舰体位置，
            // 比用 gl_FragCoord 反算省一个分辨率 uniform；clip.w 太小时（点在
            // 相机后面）保持 (0,0)，宁可圆心偏掉也不要除出 NaN。
            glm::vec2 flashCenter(0.0f);
            const glm::vec4 clip = projection * view * glm::vec4(ship.RenderPosition(alpha), 1.0f);
            if (clip.w > 1e-4f) flashCenter = glm::vec2(clip.x / clip.w, clip.y / clip.w);

            //warpFlashShader.use();
            //warpFlashShader.setFloat("uFlash", flashAmount);
            //warpFlashShader.setFloat("uFlashBase", 6.0f);
            //warpFlashShader.setFloat("uFlashCore", 10.0f);
            //warpFlashShader.setVec2("uFlashCenter", flashCenter);
            // 两者互斥：拒绝只发生在"不在折跃、不在冷却"时，白闪只在折跃期间。
            const bool red = (rejectFlash > 0.0f);
            warpFlashShader.use();
            warpFlashShader.setFloat("uFlash", red ? rejectFlash : flashAmount);
            warpFlashShader.setFloat("uFlashBase", red ? warp_tune::U_FLASH_REJECT_BASE : warp_tune::U_FLASH_BASE);
            warpFlashShader.setFloat("uFlashCore", red ? warp_tune::U_FLASH_REJECT_CORE : warp_tune::U_FLASH_CORE);
            warpFlashShader.setVec3("uFlashColor", red ? warp_tune::U_FLASH_REJECT_COLOR : warp_tune::U_FLASH_COLOR);
            warpFlashShader.setVec2("uFlashCenter", flashCenter);


            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        //==========================================
        // 渲染到屏幕

        // ===== 折跃：冲击波扭曲（实现在 WarpDebris.cpp）=====
        //  必须在 PostProcessing 之前：它把扭曲后的画面写回 hdrColorBuffer，
        //  于是后面的亮度提取（bloom）与最终合成都从扭曲后的图像走。
        {
            const bool shockOut = warp_sc::shockOutLive(gWarp.tau);
            WarpShockDistort(warpShockDistortShader, projection, view,
                shockOut ? shipWireModel : pillarPlane, windowwidth, windowheight, quadVAO);
        }

        // ===== 后处理 =====
        PostProcessing(brightPassShader, blurShader, compositeShader, quadVAO);

        // 5. 渲染镜头光晕
        Sun.LensFlareRender(lensFlareShader, camera, projection, view, flareTextures);
        // --------------------------------------------------


        // 恢复深度测试和混合状态
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 恢复默认混合函数


        // 恢复原始状态
        if (!depthEnabled) glDisable(GL_DEPTH_TEST);
        else glEnable(GL_DEPTH_TEST);
        glDepthMask(depthMask ? GL_TRUE : GL_FALSE);
        glDepthFunc(depthFunc);
        if (blendEnabled) glEnable(GL_BLEND);
        else glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 恢复默认混合函数

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &starVAO);
    glDeleteBuffers(1, &starVBO);
    glDeleteBuffers(1, &starEBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);


    glfwTerminate(); // 清理并关闭GLFW
    return 0;
}




// 窗口回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height); // glViewport 用于设置视口大小

    windowwidth = width;
    windowheight = height;

    rebuildFramebuffers(width, height); // 重新创建帧缓冲对象

    lastX = width / 2.0f;
    lastY = height / 2.0f;
}

// 输入检查函数
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 模式切换：数字行 1/2/3
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        currentMode = MODE_FREE;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        currentMode = MODE_REMOTE;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        currentMode = MODE_FOLLOW;
    // ===== 折跃：T 键触发（一次性，照抄 TAB 那套上升沿写法）=====
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !tKeyPressed)
    {
        tKeyPressed = true;

        // 三种拒绝：正在折跃、在冷却中、落点不合法（阶段 6）。前两种静默，
        // 第三种播一次红闪——那是玩家唯一能拿到的反馈。
        const bool busy = gWarp.active;
        const bool cooling = (gWarp.cooldown > 0.0f);

        if (!busy && !cooling)
        {
            glm::vec3 fhat = ship.Forward();
            const float flen = glm::length(fhat);
            if (flen > 1e-6f)
            {
                fhat /= flen;                       // 单位化，delta 才等于距离

                // 阶段 6：落点合法性。航向射线与火星球求交：|m + t d|^2 = R^2，
                // m = 起点 - 球心，近根 t_in 就是撞球的位置。三种情形要分清：球在背后
                // （两根都负）不钳；起点已在球内（t_in < 0 < t_exit）钳到 0，必然被拒；
                // 其余按 t_in 钳。
                // t_in 与 dist 都是【沿航向的位移】，可直接相减 —— 位移是整体的，不要
                // 为了"起点在舰首、落点写回中心"再补一个 gShipBowOffset（那等于把落点
                // 前移 26.8，舰首反而埋进球里）。两个余量相加后一起减，分别对应：
                //   1. 落点：让【舰首】停在球面前 5% 半径处；
                //   2. 蓄能期前冲会把画出来的舰体再往航向推最多 SLIDE_DEPART_K * gShipBowOffset，起点侧
                //      的舰首余量也必须大于它，否则蓄能那半秒舰艏会捅进球里。
                // 于是被接受的折跃满足 t_in >= WARP_MIN_DIST + 0.05 * R + SLIDE_DEPART_K * gShipBowOffset。
                const glm::vec3 m = ship.position + fhat * gShipBowOffset - gPlanetPos;
                const float b = glm::dot(m, fhat);
                const float cQ = glm::dot(m, m) - gPlanetRadius * gPlanetRadius;
                const float disc = b * b - cQ;
                float dist = WARP_DISTANCE;
                if (disc > 0.0f)
                {
                    const float sq = std::sqrt(disc);
                    if (sq - b > 0.0f)              // 只有球在射线正前方才钳
                        dist = std::min(WARP_DISTANCE,
                            std::max(0.0f, -b - sq - (0.05f * gPlanetRadius
                                + warp_sc::SLIDE_DEPART_K * gShipBowOffset)));
                    //std::max(0.0f, -b - sq - (0.05f * gPlanetRadius + gShipBowOffset)));
                }

                if (dist < WARP_MIN_DIST)
                {
                    // 落点非法：唯一反馈是一趟红闪，不消耗冷却、不启动时间线。
                    gWarp.reject = WARP_REJECT_FLASH;
                }
                else
                {
                    //gWarp.delta = fhat * dist;

                    // 落点在按键帧一次定死。传送那一帧直接用这个点，于是"从按键那一刻
                    // 算起的总位移"精确等于 dist —— 蓄能期的惯性前飘不再改变落点，
                    // 落点合法性判定与最终落点也从此是同一个量。
                    gWarp.target = ship.position + fhat * dist;
                    gWarp.tau = 0.0f;
                    gWarp.active = true;
                }
            }
        }
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
    {
        tKeyPressed = false;
    }

    // 3种模式
    if (currentMode == MODE_FREE) {
        // mode 1: WASD moves the camera
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }
    else {
        // mode 2/3: set target speeds and target angular velocities
        ship.targetSpeed = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            ship.targetSpeed = ship.forwardMax;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            ship.targetSpeed = -ship.backwardMax;

        ship.targetVerticalSpeed = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            ship.targetVerticalSpeed = ship.upMax;
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            ship.targetVerticalSpeed = -ship.downMax;

        ship.targetYawAV = 0.0f;
        ship.targetPitchAV = 0.0f;
        ship.targetRollAV = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            ship.targetYawAV = ship.turnRate;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            ship.targetYawAV = -ship.turnRate;
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            ship.targetPitchAV = ship.turnRate;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
            ship.targetPitchAV = -ship.turnRate;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            ship.targetRollAV = ship.turnRate;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            ship.targetRollAV = -ship.turnRate;


    }
    // 折跃期间冻结舰体控制。必须在三个模式分支之后独立判断，不能写成分支里的
    // else if —— MODE_FREE 下舰体本来就不受操控，那五个目标量没有任何一处会
    // 重置，会一直保留上一次在模式 2/3 里设的角速度，舰体在折跃过程中继续转，
    // heading 就漂了，落点方向与视觉方向脱钩。
    if (gWarp.active)
    {
        ship.targetSpeed = 0.0f;
        ship.targetVerticalSpeed = 0.0f;
        ship.targetYawAV = 0.0f;
        ship.targetPitchAV = 0.0f;
        ship.targetRollAV = 0.0f;
    }


    // 相机旋转
    if (currentMode == MODE_FOLLOW) {
        // mode 3: keypad orbits the camera around the ship
        if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
            orbitYaw += 50.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
            orbitYaw -= 50.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
            orbitPitch += 50.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
            orbitPitch -= 50.0f * deltaTime;
    }
    else
    {
        if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
            camera.ProcessKeyboardRotate(1.0f, deltaTime);    // 左转
        if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
            camera.ProcessKeyboardRotate(-1.0f, deltaTime);   // 右转

        if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
            camera.ProcessKeyboardPitch(1.0f, deltaTime);     // 抬头
        if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
            camera.ProcessKeyboardPitch(-1.0f, deltaTime);    // 低头

        if (glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS)
            camera.ProcessKeyboardRoll(1.0f, deltaTime);  // 顺时针
        if (glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS)
            camera.ProcessKeyboardRoll(-1.0f, deltaTime);   // 逆时针

    }



    // 限制范围
    camera.Sensitivity = glm::clamp(camera.Sensitivity, 0.01f, 0.5f);

    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabKeyPressed)
    {
        tabKeyPressed = true;
        if (cursorLocked)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            cursorLocked = false;
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            cursorLocked = true;
            firstMouse = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE)
    {
        tabKeyPressed = false;
    }

    // F10 进入全屏
    if (glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS && !f10Pressed)
    {
        f10Pressed = true;
        if (!isFullscreen)
        {
            // 保存当前窗口状态
            glfwGetWindowPos(window, &savedX, &savedY);
            glfwGetWindowSize(window, &savedWidth, &savedHeight);
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            isFullscreen = true;
            firstMouse = true; // 重置鼠标首次移动标志
        }
    }
    if (glfwGetKey(window, GLFW_KEY_F10) == GLFW_RELEASE)
        f10Pressed = false;


    // F11 退出全屏
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS && !f11Pressed)
    {
        f11Pressed = true;
        if (isFullscreen)
        {
            glfwSetWindowMonitor(window, nullptr, savedX, savedY, savedWidth, savedHeight, 0);
            isFullscreen = false;
            firstMouse = true; // 重置鼠标首次移动标志
        }
    }
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_RELEASE)
        f11Pressed = false;


    // Shadow
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS && !shadowKeyPressed)
    {
        shadows = !shadows;
        shadowKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE)
    {
        shadowKeyPressed = false;
    }

    // PCSS
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !PCSSKeyPressed)
    {
        PCSS = !PCSS;
        PCSSKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
    {
        PCSSKeyPressed = false;
    }

    // SSAO
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && !ssaoKeyPressed)
    {
        ssaoEnabled = !ssaoEnabled;
        ssaoKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE)
    {
        ssaoKeyPressed = false;
    }
    // SSAO是否采用统一角度
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS && !unifyKeyPressed)
    {
        unify = !unify;
        unifyKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_RELEASE)
    {
        unifyKeyPressed = false;
    }

}

// 鼠标回调函数
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    //  模式 3 的相机不看 Orient（跟随块直接写 Front/Right/Up），所以鼠标在这里只做无用功，
    //  而且会给"切回模式 1/2 的那一帧"留下竞态。直接跳过旋转。
    //  **!!  必须放在 lastX/lastY 更新【之后】：若在回调开头就 return，锚点会停止刷新，
    //        切回来时第一帧的 xoffset 会是从模式 3 前的旧坐标算起的一大跳。
    if (currentMode != MODE_FOLLOW)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

// 滚轮回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// 帧缓冲对象和纹理
void setupFramebuffers(int width, int height)
{
    const int samples = 4; // 多重采样样本数,与 glfwWindowHint(GLFW_SAMPLES, 4) 保持一致

    // --- 1) 创建 G-Buffer 并进行初始化---
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // position
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // normal
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // albedo+metallic
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

    // pbr (RGBA8: r=roughness, g=ao, a=emission)
    glGenTextures(1, &gPBR);
    glBindTexture(GL_TEXTURE_2D, gPBR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gPBR, 0);

    unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);

    // depth RBO
    glGenRenderbuffers(1, &gDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, gDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepthRBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "G-Buffer incomplete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // --- 2) 创建可采样的 HDR FBO（用于后处理，作为 resolve 目标） ---
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glGenTextures(1, &hdrColorBuffer);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorBuffer, 0);

    /*glGenRenderbuffers(1, &hdrDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, hdrDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrDepthRBO);*/

    //  深度改用【纹理】而不是 renderbuffer：后面冲击波的扭曲 pass 要在片元里反算世界坐标，
    //  而 renderbuffer 在着色器里采样不到。前面那段深度 blit 对纹理附件同样有效。
    glGenTextures(1, &hdrDepthTex);
    glBindTexture(GL_TEXTURE_2D, hdrDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);   // 当普通纹理采样，不做阴影比较
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, hdrDepthTex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "HDR FBO incomplete!" << std::endl;


    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Pingpong FBO incomplete!" << i << "incomplete" << std::endl;
    }

    // --- 3) SSAO 帧缓冲（单通道 GL_RED） ---
    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;

    // --- 4) SSAO 模糊帧缓冲 ---
    glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 重新创建帧缓冲对象
void rebuildFramebuffers(int width, int height)
{
    // 安全解绑
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // 删除旧的帧缓冲和纹理

    // 删除 MSAA 资源
    if (msFBO) { glDeleteFramebuffers(1, &msFBO); msFBO = 0; }
    if (msColorRBO) { glDeleteRenderbuffers(1, &msColorRBO); msColorRBO = 0; }
    if (msDepthRBO) { glDeleteRenderbuffers(1, &msDepthRBO); msDepthRBO = 0; }

    // 删除原有 HDR / blur 资源
    glDeleteFramebuffers(1, &hdrFBO);
    glDeleteTextures(1, &hdrColorBuffer);
    glDeleteRenderbuffers(1, &hdrDepthRBO);
    if (hdrDepthTex) { glDeleteTextures(1, &hdrDepthTex); hdrDepthTex = 0; }

    // 删除 ping-pong 资源
    if (pingpongFBO[0]) { glDeleteFramebuffers(1, &pingpongFBO[0]);     pingpongFBO[0] = 0; }
    if (pingpongFBO[1]) { glDeleteFramebuffers(1, &pingpongFBO[1]);     pingpongFBO[1] = 0; }
    if (pingpongColorbuffers[0]) { glDeleteTextures(1, &pingpongColorbuffers[0]); pingpongColorbuffers[0] = 0; }
    if (pingpongColorbuffers[1]) { glDeleteTextures(1, &pingpongColorbuffers[1]); pingpongColorbuffers[1] = 0; }

    // 删除 G-Buffer 资源
    if (gBuffer) { glDeleteFramebuffers(1, &gBuffer);     gBuffer = 0; }
    if (gPosition) { glDeleteTextures(1, &gPosition);       gPosition = 0; }
    if (gNormal) { glDeleteTextures(1, &gNormal);         gNormal = 0; }
    if (gAlbedo) { glDeleteTextures(1, &gAlbedo);     gAlbedo = 0; }
    if (gDepthRBO) { glDeleteRenderbuffers(1, &gDepthRBO);  gDepthRBO = 0; }
    if (gPBR) { glDeleteTextures(1, &gPBR);          gPBR = 0; }

    // 删除 SSAO 资源
    if (ssaoFBO) { glDeleteFramebuffers(1, &ssaoFBO); ssaoFBO = 0; }
    if (ssaoBlurFBO) { glDeleteFramebuffers(1, &ssaoBlurFBO); ssaoBlurFBO = 0; }
    if (ssaoColorBuffer) { glDeleteTextures(1, &ssaoColorBuffer); ssaoColorBuffer = 0; }
    if (ssaoColorBufferBlur) { glDeleteTextures(1, &ssaoColorBufferBlur); ssaoColorBufferBlur = 0; }

    // 重新创建帧缓冲
    setupFramebuffers(width, height);
}

// 小行星带初始化
void RocksModelMatricesInit(unsigned int& amount, Model& rock)
{
    float rockMeshBoundR = 0.0f;
    for (auto& m : rock.meshes)
        for (auto& v : m.vertices)
            rockMeshBoundR = std::max(rockMeshBoundR, glm::length(v.Position));


    glm::mat4* modelMatrices;
    modelMatrices = new glm::mat4[amount];
    srand(static_cast<unsigned int>(glfwGetTime())); // 生成随机种子
    float radius = 200.0;
    float offset = 20.0f;
    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1.  平移  ：沿圆周位移，'半径'在范围内 [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.00f - offset;
        float x = sin(angle) * radius + displacement;

        displacement = (rand() % (int)(2 * offset * 100)) / 100.00f - offset;
        float y = displacement * 0.1f; // 保持场地的高度小于x轴和z轴的宽度。

        displacement = (rand() % (int)(2 * offset * 100)) / 100.00f - offset;
        float z = cos(angle) * radius + displacement;

        model = glm::translate(model, glm::vec3(x, y, z));

        // 2.  缩放  ：在 0.05 和 0.25f 之间进行缩放 
        float scale = static_cast<float>((rand() % 30) / 100.0 + 0.01); // 在 0.05 和 0.17f 之间缩放，使得小行星更小一些
        // 添加尺寸变化以模拟更真实的小行星带
        if (i % 200 == 0)
        {
            scale *= 6.0f; // 每200个小行星中有一个更大一些
        }
        else if (i % 20 == 0)
        {
            scale *= 3.0f; // 每20个小行星中有一个中等大小
        }
        else if (i % 5 == 0)
        {
            scale *= 1.5f; // 每5个小行星中有一个稍微大一些
        }
        model = glm::scale(model, glm::vec3(scale));

        // 3.  旋转  ：围绕一个（半）随机选取的旋转轴向量进行随机旋转。
        float rotAngle = static_cast<float>((rand() % 360));
        //model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
            // 更复杂的旋转 - 多个旋转轴组合
        glm::mat4 rotation = glm::mat4(1.0f);
        rotation = glm::rotate(rotation, rotAngle * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
        rotation = glm::rotate(rotation, rotAngle * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        rotation = glm::rotate(rotation, rotAngle * 0.2f, glm::vec3(0.0f, 0.0f, 1.0f));
        model = model * rotation;

        // 4. 现在添加到矩阵列表中
        modelMatrices[i] = model;

        gRockMatrices.push_back(model);
        //float s = glm::length(glm::vec3(model[0][0], model[1][1], model[2][2]));    // 均匀缩放
        // 改成：取任一列向量的长度
        //float s = glm::length(glm::vec3(model[0]));
        // 对以后可能出现的非均匀缩放也安全，就取三列最大值
        float s = glm::max(glm::length(glm::vec3(model[0])),
            glm::max(glm::length(glm::vec3(model[1])), glm::length(glm::vec3(model[2]))));
        // 测试边角易出现剔除问题的小行星
        static int badR = 0; static float fMin = 9e9f;
        float f = s / scale;                 // scale 是 L1313 那个真实缩放
        fMin = glm::min(fMin, f);
        if (f < 0.6f) { badR++; std::cout << "[radius] i=" << i << " rot=" << rotAngle << " f=" << f << "\n"; }
        // 循环外打印一次 fMin / badR

        gRockSpheres.push_back(glm::vec4(glm::vec3(model[3]), rockMeshBoundR * s));
    }

    // 设置实例化顶点属性
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    rockInstanceVBO = buffer;                                   // 存全局供每帧重填
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    //glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
    //glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::mat4), gRockMatrices.data());
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

    // 将变换矩阵设置为实例顶点属性（使用除数1）
    // 注意：我们这里有点取巧，直接获取模型网格（多个网格时）现在公开声明的VAO，并添加新的vertexAttribPointers
    // 正常情况下，你会希望以更有条理的方式来做这件事，但出于学习目的，这样做就可以了。
    // 
    for (unsigned int i = 0; i < rock.meshes.size(); i++)
    {
        unsigned int VAO = rock.meshes[i].VAO;
        glBindVertexArray(VAO);
        // 设置顶点属性指针
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }

    // 删除
    delete[] modelMatrices;
}

// 帧缓冲四边形初始化
void FrameQuadInit(unsigned int& quadVAO, unsigned int& quadVBO)
{
    float quadVertices[] = {
        // 位置(x,y)      纹理坐标(u,v)
        -1.0f,  1.0f,     0.0f, 1.0f,
        -1.0f, -1.0f,     0.0f, 0.0f,
         1.0f, -1.0f,     1.0f, 0.0f,
        -1.0f,  1.0f,     0.0f, 1.0f,
         1.0f, -1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,     1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

// 阴影贴图初始化
void DepthCubeMapInit()
{
    glGenFramebuffers(1, &depthCubeFBO);
    glGenTextures(1, &depthCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
    for (unsigned int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, depthCubeFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ---- 动态层 cubemap (只装会动的物体：飞船) ----
    glGenFramebuffers(1, &depthDynFBO);
    glGenTextures(1, &depthDynMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthDynMap);
    for (unsigned int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            DYN_SHADOW_SIZE, DYN_SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, depthDynFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthDynMap, 0);  // 分层附件
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
}

// 阴影PASS渲染
void ShadowPassRender(glm::mat4& shadowProj, std::vector<glm::mat4>& shadowTransforms, const glm::vec3& pointSunPositions)
{
    /*glm::mat4 shadowProj = glm::perspective(
        glm::radians(90.0f),
        (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT,
        shadow_near, shadow_far);*/

        //std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointSunPositions, pointSunPositions + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointSunPositions, pointSunPositions + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointSunPositions, pointSunPositions + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointSunPositions, pointSunPositions + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointSunPositions, pointSunPositions + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(pointSunPositions, pointSunPositions + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthCubeFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_BACK);
}

// 几何所在的那个立方体面 (0=+X,1=-X,2=+Y,3=-Y,4=+Z,5=-Z)
int StaticShadowFace(const glm::vec3& lightPos, const glm::vec3& center)
{
    glm::vec3 d = glm::normalize(center - lightPos);
    glm::vec3 a = glm::abs(d);
    if (a.x >= a.y && a.x >= a.z) return (d.x > 0.0f) ? 0 : 1;
    if (a.y >= a.z)               return (d.y > 0.0f) ? 2 : 3;
    return (d.z > 0.0f) ? 4 : 5;
}

// 开始一次阴影渲染：建 6 个面矩阵 + 绑 FBO + 清空(分层附件一次清 6 面)
void ShadowPassBegin(unsigned int fbo, int res, glm::mat4& shadowProj,
    std::vector<glm::mat4>& shadowTransforms, const glm::vec3& lightPos)
{
    shadowTransforms.clear();
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

    glViewport(0, 0, res, res);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glDepthMask(GL_TRUE);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);   // 分层附件 → 一次清掉全部 6 面（未用面 = 最远 = 无遮挡）
    glCullFace(GL_BACK);
}


// SSAO初始化
void SSAOInit()
{
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
    std::default_random_engine generator;

    // 1. 生成 64 个切线空间半球采样点
    ssaoKernel.clear();
    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = (float)i / 64.0f;
        scale = 0.1f + 0.9f * (scale * scale);   // lerp(0.1, 1.0, scale²)
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // 2. 生成 4x4 随机旋转噪声纹理
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator) * 2.0f - 1.0f,
            0.0f);
        ssaoNoise.push_back(noise);
    }
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //// 3. SSAO 帧缓冲（单通道 GL_RED）
    //glGenFramebuffers(1, &ssaoFBO);
    //glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    //glGenTextures(1, &ssaoColorBuffer);
    //glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowwidth, windowheight, 0, GL_RED, GL_FLOAT, NULL);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    //if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    //    std::cout << "SSAO Framebuffer not complete!" << std::endl;

    //// 4. SSAO 模糊帧缓冲
    //glGenFramebuffers(1, &ssaoBlurFBO);
    //glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    //glGenTextures(1, &ssaoColorBufferBlur);
    //glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowwidth, windowheight, 0, GL_RED, GL_FLOAT, NULL);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    //if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    //    std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RingGenerate(float outerRadius, float thickness)
{
    float hx = outerRadius;
    float hy = thickness * 0.5f;
    float hz = outerRadius;

    float vertices[] = {
        -hx, -hy,  hz,   // 0 前下左
         hx, -hy,  hz,   // 1 前下右
         hx,  hy,  hz,   // 2 前上右
        -hx,  hy,  hz,   // 3 前上左
        -hx, -hy, -hz,   // 4 后下左
         hx, -hy, -hz,   // 5 后下右
         hx,  hy, -hz,   // 6 后上右
        -hx,  hy, -hz,   // 7 后上左
    };

    unsigned int indices[] = {
        0,1,2, 0,2,3,    // 前 +Z
        5,4,7, 5,7,6,    // 后 -Z
        1,5,6, 1,6,2,    // 右 +X
        4,0,3, 4,3,7,    // 左 -X
        3,2,6, 3,6,7,    // 上 +Y
        4,5,1, 4,1,0,    // 下 -Y
    };

    ringIndexCount = 36;

    glGenVertexArrays(1, &ringVAO);
    glGenBuffers(1, &ringVBO);
    glGenBuffers(1, &ringEBO);

    glBindVertexArray(ringVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ringVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ringEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void RockViewFrustumCull(GLFWwindow* window, const glm::vec3& lightPos)
{
    // ---- 小行星视锥剔除（Option A，单一相机视锥）----
    int cw, ch;
    glfwGetFramebufferSize(window, &cw, &ch);
    float cAspect = (float)cw / (float)ch;
    glm::mat4 cullProj = glm::perspective(glm::radians(camera.Fov), cAspect, 0.1f, 2000.0f);
    glm::mat4 cullView = camera.GetViewMatrix();
    FrustumPlanes fp = ExtractFrustum(cullProj * cullView);

    gVisible.clear();
    gShadowVisible.clear();
    // 新增
    for (int f = 0; f < 6; ++f) { gFaceCasters[f].clear(); gFaceCount[f] = 0; }


    for (size_t i = 0; i < gRockSpheres.size(); ++i)
    {
        glm::vec3 c = glm::vec3(gRockSpheres[i]);
        float r = gRockSpheres[i].w;        // 半径（vec4 的 w 分量）

        // A: G-Buffer 用
        if (SphereInFrustum(fp, c, r))              // 如果包围球和视锥相交       
        {
            gVisible.push_back(gRockMatrices[i]);   // 将该实例的变换矩阵添加到可见列表（用于实例化绘制到 G-Buffer）
        }

        // C：阴影候选
        if (CanCastVisibleShadow(fp, lightPos, c, r))  // 如果该对象可能对当前光源产生可见阴影（阴影候选）
        {
            gShadowVisible.push_back(gRockMatrices[i]); // 将其添加到阴影候选列表

            int faces[6];
            int n = AssignCasterFaces(lightPos, c, r, faces); // 计算该球体影响到的深度立方体面（返回面数量并写入 faces）
            for (int k = 0; k < n; ++k)
                gFaceCasters[faces[k]].push_back(gRockMatrices[i]); // 将该矩阵加入对应面的投射列表（便于按面渲染阴影）
        }

    }
    for (int f = 0; f < 6; ++f)
        gFaceCount[f] = (unsigned int)gFaceCasters[f].size(); // 更新每个面的投射体计数

    rockVisibleCount = (unsigned int)gVisible.size();           // 可见小行星实例总数（用于绘制实例计数）
    rockShadowVisibleCount = (unsigned int)gShadowVisible.size(); // 阴影候选总数


    // ---- 自测校验（临时，跑前几帧打印；确认后删除或包 #ifdef DEBUG_CULL）----
    static int dbgCullFrame = 0;
    if (++dbgCullFrame <= 3) {
        glm::vec3 fwd = glm::normalize(camera.Front);
        glm::vec3 cIn = glm::vec3(camera.Position) + fwd * 10.0f;
        glm::vec3 cOut = glm::vec3(camera.Position) - fwd * 1000.0f;

        std::cout << "[cull] center-in=" << SphereInFrustum(fp, cIn, 1.0f)
            << " behind-out=" << (!SphereInFrustum(fp, cOut, 1.0f))
            << " visibleCount=" << rockVisibleCount << " / " << gRockSpheres.size() << "\n";

        std::cout << "[shadow] f0=" << gFaceCount[0] << " f1=" << gFaceCount[1]
            << " f2=" << gFaceCount[2] << " f3=" << gFaceCount[3]
            << " f4=" << gFaceCount[4] << " f5=" << gFaceCount[5]
            << " total=" << rockShadowVisibleCount << "\n";
    }
}




#endif