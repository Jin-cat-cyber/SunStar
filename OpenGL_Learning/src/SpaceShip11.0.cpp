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

//#ifdef SHIP_11_0
#include <stb_image.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);  // 窗口大小回调函数
void processInput(GLFWwindow* window);  // 输入检查函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos);  // 鼠标 移动 回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);   // 鼠标 滚轮 回调函数
void setupFramebuffers(int eidth, int height);  //  离屏渲染帧缓冲
void rebuildFramebuffers(int width, int height);  //  重建离屏渲染帧缓冲



// 星球相关函数
void RocksModelMatricesInit(unsigned int& amount, Model& rock);

// 阴影相关函数
void DepthCubeMapInit();
void ShadowPassRender(glm::mat4& shadowProj, std::vector<glm::mat4>& shadowTransforms, const glm::vec3& pointSunPositions);

// SSAO相关函数
void SSAOInit();


// 新增多重采样 FBO 句柄
unsigned int msFBO = 0;
unsigned int msColorRBO = 0;
unsigned int msDepthRBO = 0;



// 后处理四边形顶点数组对象和顶点缓冲对象
void FrameQuadInit(unsigned int& quadVAO, unsigned int& quadVBO);




int main()
{
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

    // 延迟光照
    Shader deferredLightingShader("res/shader/00_SpaceShip/Deferred_Shading2.0/deferred_lighting_ver.shader",
        "res/shader/00_SpaceShip/Deferred_Shading2.0/defer_light_ssao_frag.shader");

    // Spaceship
    Shader spaceshipShader("res/shader/00_SpaceShip/Forward_Shading/spaceship_ver.shader",
        "res/shader/00_SpaceShip/Forward_Shading/spaceship_frag.shader");
    // 火星
    Shader MarsShader("res/shader/00_SpaceShip/Forward_Shading/planet_ver.shader",
        "res/shader/00_SpaceShip/Forward_Shading/planet_frag.shader");

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
    
    // LensFlare
    Shader lensFlareShader("res/shader/LensFlareShader/lens_flare_ver.shader",
        "res/shader/LensFlareShader/lens_flare_frag.shader");

    // Shadow
    Shader simpleDepthShader("res/shader/00_SpaceShip/depth_point/depth_point_ver2.2.shader",
        "res/shader/00_SpaceShip/depth_point/depth_point_frag.shader",
        "res/shader/00_SpaceShip/depth_point/depth_point_geo.shader");


    Model rock("res/model/rock/rock.obj");
    PbrModel planet("res/model/glb_model/planet/mars_2k.glb");

    PbrModel spaceship("res/model/glb_model/homeworld_-_vaygr_battlecruiser_1k.glb");
    


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



    // 设置恒星，星球位置和大小
    glm::vec3 pointSunPositions = glm::vec3(-50.0f, 50.0f, -600.0f);
    //glm::vec3 SunScale = glm::vec3(120.0f);
    glm::vec3 planetPosition = glm::vec3(0.0f, -3.0f, 0.0f);
    glm::vec3 planetScale = glm::vec3(0.8f);

    // 激活着色器纹理单元
    //planetshader.use();
    //planetshader.setInt("material.diffuse", 0); // 告诉着色器每个采样器属于哪个纹理单元(只需要设置一次)
    //planetshader.setInt("material.specular", 1);


    // pbr: setup framebuffer
    // ----------------------
    unsigned int captureFBO;
    unsigned int captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 2048, 2048);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);


    // pbr: load the HDR environment map
    // ---------------------------------
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    //float* data = stbi_loadf("res/texture/hdr/newport_loft.hdr", &width, &height, &nrComponents, 0);
    float* data = stbi_loadf("res/texture/hdr/space_fox.hdr", &width, &height, &nrComponents, 0);

    unsigned int hdrTexture;
    if (data)
    {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load HDR image." << std::endl;
    }

    // pbr: setup cubemap to render to and attach to framebuffer
    // ---------------------------------------------------------
    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 2048, 2048, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
    // ----------------------------------------------------------------------------------------------
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // pbr: convert HDR equirectangular environment map to cubemap equivalent
    // ----------------------------------------------------------------------
    equirectangularToCubemapShader.use();
    equirectangularToCubemapShader.setInt("equirectangularMap", 0);
    equirectangularToCubemapShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 2048, 2048); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        equirectangularToCubemapShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
    // --------------------------------------------------------------------------------
    unsigned int irradianceMap;
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    //*** pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
    // -----------------------------------------------------------------------------
    irradianceShader.use();
    irradianceShader.setInt("environmentMap", 0);
    irradianceShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        irradianceShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //*** pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
   // --------------------------------------------------------------------------------
    unsigned int prefilterMap;
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    //*** pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
    // ----------------------------------------------------------------------------------------------------
    prefilterShader.use();
    prefilterShader.setInt("environmentMap", 0);
    prefilterShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader.setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            prefilterShader.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //*** pbr: generate a 2D LUT from the BRDF equations used.
    // ----------------------------------------------------
    unsigned int brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    brdfShader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 天空盒改用 IBL 生成的 envCubemap（天空与光照环境统一）
    Skybox SpaceBox(skyboxVAO, skyboxVBO, envCubemap);

    // then before rendering, configure the viewport to the original framebuffer's screen dimensions
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);


    glm::vec3 spaceshipPosition = glm::vec3(-100.0f, 40.0f, 60.0f);

    // 主循环
    while (!glfwWindowShouldClose(window))
    {



        // ====== 阴影 Pass：渲染深度 CubeMap ======
        std::vector<glm::mat4> shadowTransforms;
        glm::mat4 shadowProj = glm::perspective(
            glm::radians(90.0f),
            (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT,
            shadow_near, shadow_far);

        ShadowPassRender(shadowProj, shadowTransforms, pointSunPositions);


        simpleDepthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            simpleDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        simpleDepthShader.setFloat("far_plane", shadow_far);
        simpleDepthShader.setVec3("lightPos", pointSunPositions);

        // --- 小行星带 ---
        simpleDepthShader.setBool("instanced", true);
        for (unsigned int i = 0; i < rock.meshes.size(); i++)
        {
            glBindVertexArray(rock.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES,
                static_cast<unsigned int>(rock.meshes[i].indices.size()),
                GL_UNSIGNED_INT, 0, amount);
            glBindVertexArray(0);
        }


        // --- 星球 ---
        simpleDepthShader.setBool("instanced", false);
        glm::mat4 sdModel = glm::mat4(1.0f);
        sdModel = glm::translate(sdModel, planetPosition);
        sdModel = glm::scale(sdModel, planetScale);
        simpleDepthShader.setMat4("model", sdModel);
        planet.Draw(simpleDepthShader);

        // --- 飞船（向深度 Cubemap 投影）---
        glm::mat4 spaceshipModel = glm::mat4(1.0f);
        spaceshipModel = glm::translate(spaceshipModel, spaceshipPosition);
        spaceshipModel = glm::rotate(spaceshipModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        spaceshipModel = glm::rotate(spaceshipModel, glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        spaceshipModel = glm::rotate(spaceshipModel, glm::radians(185.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        spaceshipModel = glm::scale(spaceshipModel, glm::vec3(0.0005f));
        simpleDepthShader.setBool("instanced", false);
        simpleDepthShader.setMat4("model", spaceshipModel);
        spaceship.Draw(simpleDepthShader);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCullFace(GL_BACK);
        // ====== 阴影 Pass 结束 ======


        glViewport(0, 0, windowwidth, windowheight);


        // 计算帧时间
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 输入管理
        processInput(window);
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
        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), aspect, 0.1f, 2000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // === 几何 Pass: Planet === (火星改前向渲染，不再进 G-Buffer)
        //gBufferPlanetShader.use();
        //gBufferPlanetShader.setMat4("projection", projection);
        //gBufferPlanetShader.setMat4("view", view);
        //glm::mat4 model = glm::mat4(1.0f);
        //model = glm::translate(model, planetPosition);
        //model = glm::scale(model, planetScale);
        //gBufferPlanetShader.setMat4("model", model);
        //planet.Draw(gBufferPlanetShader);

        // === 几何 Pass: Asteroids ===
        gBufferAsteroidShader.use();
        gBufferAsteroidShader.setMat4("projection", projection);
        gBufferAsteroidShader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rock.textures_loaded[0].id);
        gBufferAsteroidShader.setInt("material_diffuse", 0);
        for (unsigned int i = 0; i < rock.meshes.size(); i++)
        {
            glBindVertexArray(rock.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES,
                static_cast<unsigned int>(rock.meshes[i].indices.size()),
                GL_UNSIGNED_INT, 0, amount);
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

        glCullFace(GL_BACK);
        planet.Draw(MarsShader);

        // ====== 飞船 forward PBR 渲染 ======
        glDepthMask(GL_TRUE);

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
        spaceshipShader.setFloat("rimStrength", 0.6f);                     // 强度 

        spaceshipShader.setBool("shadows", true);
        spaceshipShader.setBool("PCSS", false);
        spaceshipShader.setFloat("far_plane", shadow_far);

        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
        glActiveTexture(GL_TEXTURE9); glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);

        glCullFace(GL_BACK);
        spaceship.Draw(spaceshipShader);


        //==========================================
        // 渲染到屏幕
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        // 1. 亮度提取：从hdrColorBuffer中提取亮度信息到blurFBO1
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
        glClear(GL_COLOR_BUFFER_BIT); // 这个为什么只清理颜色缓冲?
        brightPassShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
        brightPassShader.setInt("hdrImage", 0);
        brightPassShader.setFloat("threshold", 1.2f); // 设置亮度阈值 1.2f
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 2. 乒乓高斯模糊
        int blurPasses = 10;         // 6~10 越大越柔和
        bool horizontal = true;
        blurShader.use();
        for (int i = 0; i < blurPasses; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setBool("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
            blurShader.setInt("image", 0);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            horizontal = !horizontal;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //// MSAA帧缓冲到HDR帧缓冲的blit操作
        //glBindFramebuffer(GL_READ_FRAMEBUFFER, msFBO);
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO);
        //glBlitFramebuffer(0, 0, windowwidth, windowheight,
        //    0, 0, windowwidth, windowheight,
        //    GL_COLOR_BUFFER_BIT, GL_NEAREST);

        //// 【新增】把深度从 msFBO 复制到默认帧缓冲，供后面的光晕使用
        //glBindFramebuffer(GL_READ_FRAMEBUFFER, msFBO);
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        //glBlitFramebuffer(0, 0, windowwidth, windowheight,
        //    0, 0, windowwidth, windowheight,
        //    GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        //// 恢复为默认帧缓冲或绑定 hdrFBO 以便后续读取 hdrColorBuffer
        //glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

        // ------- 合成到屏幕 -------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);   // 回到默认帧缓冲
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // 老版本
        // 只清理颜色缓冲，不清理深度缓冲，以此来保留深度信息，确保后续渲染的物体不会被清除
        glClear(GL_COLOR_BUFFER_BIT);
        compositeShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
        compositeShader.setInt("sceneTexture", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]); // 最终模糊结果在 blurFBO1 的颜色附件
        compositeShader.setInt("bloomTexture", 1);
        compositeShader.setFloat("exposure", 1.0f); // 曝光值
        compositeShader.setFloat("bloomStrength", 0.6f); // Bloom强度
        compositeShader.setVec3("colorTint", glm::vec3(1.0f, 0.95f, 0.6f)); // Bloom颜色
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // blit hdrFBO depth to default FB for lens flare
        glBindFramebuffer(GL_READ_FRAMEBUFFER, hdrFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, windowwidth, windowheight,
            0, 0, windowwidth, windowheight,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        // 5. 渲染镜头光晕
        Sun.LensFlareRender(lensFlareShader, camera, projection, view, flareTextures);

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

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.Sensitivity += 0.001f; // 增加鼠标灵敏度
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.Sensitivity -= 0.001f; // 减少鼠标灵敏度

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboardRotate(1.0f, deltaTime);    // 逆时针 (左转)
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboardRotate(-1.0f, deltaTime);   // 顺时针 (右转)

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        camera.ProcessKeyboardPitch(1.0f, deltaTime);     // 抬头
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        camera.ProcessKeyboardPitch(-1.0f, deltaTime);    // 低头

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


    //bool shadowKeyPressed = false;
    //bool PCSSKeyPressed = false;
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// 滚轮回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


// 帧缓冲对象和纹理
//unsigned int hdrFBO, blurFBO1, blurFBO2;
//unsigned int hdrColorBuffer, blurColorBuffer1, blurColorBuffer2;
//unsigned int hdrDepthRBO;
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

    glGenRenderbuffers(1, &hdrDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, hdrDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrDepthRBO);
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
    }

    // 设置实例化顶点属性
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

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

// renderCube() 渲染一个位于 NDC 中的 1x1 3D 立方体。
// -----------------------------------------


//#endif