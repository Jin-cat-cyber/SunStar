//#include <glad/glad.h>
//#include <glfw3.h>
//#include <assimp/config.h>
//#include <assimp/revision.h>
//
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>
//
//#include <iostream>
//#include <string>
//#include <vector>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//
//#include "camera_ver2.h"
//#include "Model.h"
//#include "Shader.h" // 包含自定义着色器类
//
//
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height);  // 窗口大小回调函数
//void processInput(GLFWwindow* window);  // 输入检查函数
//void mouse_callback(GLFWwindow* window, double xpos, double ypos);  // 鼠标 移动 回调函数
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);   // 鼠标 滚轮 回调函数
//unsigned int loadTexture(char const* path);     // 纹理加载函数
//unsigned int loadCubemap(std::vector<std::string> faces);   // 立方体贴图加载函数
//void setupFramebuffers(int eidth, int height);  //  离屏渲染帧缓冲
//
//const unsigned int SCR_WIDTH = 960; // 窗口宽度
//const unsigned int SCR_HEIGHT = 600; // 窗口高度
//
//// 摄像机相关
//Camera_ver2 camera(glm::vec3(-40.0f, 10.0f, 200.0f));
//float lastX = SCR_WIDTH / 2.0F;
//float lastY = SCR_HEIGHT / 2.0F;
//bool firstMouse = true;
//bool cursorLocked = true;
//bool tabKeyPressed = false;   // 用于检测 TAB 键的上升沿
//
//
//// 设置帧数渲染时间
//float deltaTime = 0.0f;	// 当前帧与上一帧的时间差
//float lastFrame = 0.0f; // 上一帧的时间
//
//// 设置光源位置
//glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
//
//// 后处理帧缓冲变量
//unsigned int hdrFBO, blurFBO1, blurFBO2;
//unsigned int hdrColorBuffer, blurColorBuffer1, blurColorBuffer2;
//unsigned int hdrDepthRBO;
//
//
//int main()
//{
//    glfwInit(); // 初始化GLFW库
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // 设置OpenGL版本：主版本号
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // 设置OpenGL版本：次版本号
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 使用核心模式
//    glfwWindowHint(GLFW_SAMPLES, 4);
//
//    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//
//    // 创建窗口对象
//    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hello OpenGL", NULL, NULL);
//    if (window == NULL)
//    {
//        std::cout << "Failed to create GLFW window" << std::endl;
//        glfwTerminate(); // 清理窗口资源
//        return -1;
//    }
//    glfwMakeContextCurrent(window); // 设置当前窗口为上下文
//    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // 设置窗口大小回调函数
//    glfwSetCursorPosCallback(window, mouse_callback); // 设置鼠标移动回调函数
//    glfwSetScrollCallback(window, scroll_callback); // 设置鼠标滚轮回调函数
//
//    // 捕获鼠标（隐藏鼠标光标，并提供无限的鼠标移动）
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//
//    // 初始化GLAD，管理OpenGL函数指针，加载所有OpenGL函数指针
//    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//    {
//        std::cout << "Failed to initialize GLAD" << std::endl;
//        return -1;
//    }
//
//    // 纹理y轴翻转(因为OpenGL的y轴坐标是从下往上，而图片的y轴坐标是从上往下)
//    stbi_set_flip_vertically_on_load(true);
//
//    //加载深度缓冲
//    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_MULTISAMPLE); // 启用多重采样抗锯齿
//	setupFramebuffers(SCR_WIDTH, SCR_HEIGHT); // 设置离屏渲染帧缓冲
//
//    // 创建着色器对象
//    Shader planetshader("res/shader/InstancingShader/instancingVER.shader", "res/shader/InstancingShader/instancingFRAG.shader");
//    Shader asteroidShader("res/shader/InstancingShader/aster_ver.shader", "res/shader/InstancingShader/aster_frag.shader");
//
//    //Shader sunCoreShader("res/shader/StarShader/StarList/star_core_ver.shader", "res/shader/StarShader/StarList/star_core_frag.shader");
//    Shader sunCoreShader("res/shader/StarShader/StarList2.0/core_ver.shader", "res/shader/StarShader/StarList2.0/core_frag.shader");
//    Shader CoreCoronaShader("res/shader/StarShader/StarList2.0/corona_ver.shader", "res/shader/StarShader/StarList2.0/corona_frag.shader");
//    Shader sunCoronaShader("res/shader/StarShader/StarList2.0/corona_quad_ver.shader", "res/shader/StarShader/StarList2.0/corona_quad_frag.shader");
//    Shader sunGlowShader("res/shader/StarShader/StarList/star_glow_ver.shader", "res/shader/StarShader/StarList/star_glow_frag.shader");
//
//    Shader spaceboxShader("res/shader/SkyBoxShader/SkyBox_ver.shader", "res/shader/SkyBoxShader/SkyBox_frag.shader");
//
//	Shader brightPassShader("res/shader/BloomShaders/bright_pass_ver.shader", "res/shader/BloomShaders/bright_pass_frag.shader");
//	Shader blurShader("res/shader/BloomShaders/blur_ver.shader", "res/shader/BloomShaders/blur_frag.shader");
//	Shader compositeShader("res/shader/BloomShaders/composite_ver.shader", "res/shader/BloomShaders/composite_frag.shader");
//
//    Model rock("res/model/rock/rock.obj");
//    Model planet("res/model/planet/planet.obj");
//
//
//
//    // 生成一个大型的半随机模型变换矩阵列表
//    // ------------------------------------------------------------------
//    unsigned int amount = 50000;
//    glm::mat4* modelMatrices;
//    modelMatrices = new glm::mat4[amount];
//    srand(static_cast<unsigned int>(glfwGetTime())); // 生成随机种子
//    float radius = 200.0;
//    float offset = 20.0f;
//    for (unsigned int i = 0; i < amount; i++)
//    {
//        glm::mat4 model = glm::mat4(1.0f);
//        // 1.  平移  ：沿圆周位移，'半径'在范围内 [-offset, offset]
//        float angle = (float)i / (float)amount * 360.0f;
//        float displacement = (rand() % (int)(2 * offset * 100)) / 100.00f - offset;
//        float x = sin(angle) * radius + displacement;
//
//        displacement = (rand() % (int)(2 * offset * 100)) / 100.00f - offset;
//        float y = displacement * 0.1f; // 保持场地的高度小于x轴和z轴的宽度。
//
//        displacement = (rand() % (int)(2 * offset * 100)) / 100.00f - offset;
//        float z = cos(angle) * radius + displacement;
//
//        model = glm::translate(model, glm::vec3(x, y, z));
//
//        // 2.  缩放  ：在 0.05 和 0.25f 之间进行缩放 
//        float scale = static_cast<float>((rand() % 30) / 100.0 + 0.01); // 在 0.05 和 0.17f 之间缩放，使得小行星更小一些
//        // 添加尺寸变化以模拟更真实的小行星带
//        if (i % 200 == 0)
//        {
//            scale *= 6.0f; // 每200个小行星中有一个更大一些
//        }
//        else if (i % 20 == 0)
//        {
//            scale *= 3.0f; // 每20个小行星中有一个中等大小
//        }
//        else if (i % 5 == 0)
//        {
//            scale *= 1.5f; // 每5个小行星中有一个稍微大一些
//        }
//        model = glm::scale(model, glm::vec3(scale));
//
//        // 3.  旋转  ：围绕一个（半）随机选取的旋转轴向量进行随机旋转。
//        float rotAngle = static_cast<float>((rand() % 360));
//        //model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
//            // 更复杂的旋转 - 多个旋转轴组合
//        glm::mat4 rotation = glm::mat4(1.0f);
//        rotation = glm::rotate(rotation, rotAngle * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
//        rotation = glm::rotate(rotation, rotAngle * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
//        rotation = glm::rotate(rotation, rotAngle * 0.2f, glm::vec3(0.0f, 0.0f, 1.0f));
//        model = model * rotation;
//
//        // 4. 现在添加到矩阵列表中
//        modelMatrices[i] = model;
//    }
//
//    // 设置实例化顶点属性
//    unsigned int buffer;
//    glGenBuffers(1, &buffer);
//    glBindBuffer(GL_ARRAY_BUFFER, buffer);
//    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
//
//    // 将变换矩阵设置为实例顶点属性（使用除数1）
//    // 注意：我们这里有点取巧，直接获取模型网格（多个网格时）现在公开声明的VAO，并添加新的vertexAttribPointers
//    // 正常情况下，你会希望以更有条理的方式来做这件事，但出于学习目的，这样做就可以了。
//    // 
//    for (unsigned int i = 0; i < rock.meshes.size(); i++)
//    {
//        unsigned int VAO = rock.meshes[i].VAO;
//        glBindVertexArray(VAO);
//        // 设置顶点属性指针
//        glEnableVertexAttribArray(3);
//        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
//        glEnableVertexAttribArray(4);
//        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
//        glEnableVertexAttribArray(5);
//        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
//        glEnableVertexAttribArray(6);
//        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
//
//        glVertexAttribDivisor(3, 1);
//        glVertexAttribDivisor(4, 1);
//        glVertexAttribDivisor(5, 1);
//        glVertexAttribDivisor(6, 1);
//
//        glBindVertexArray(0);
//    }
//
//
//    // =======================================================
//    // 程序化生成恒星顶点数据
//    // 球体顶点数据
//    std::vector<float> starVertices;
//    std::vector<unsigned int> starIndices;
//    // 生成球体顶点数据（可以使用UV球体或其他方法）
//    const unsigned int X_SEGMENTS = 64;
//    const unsigned int Y_SEGMENTS = 64;
//    const float PI = 3.14159265359f;
//
//    // 生成球体顶点数据
//    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
//    {
//        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
//        {
//            float xSegment = (float)x / (float)X_SEGMENTS;
//            float ySegment = (float)y / (float)Y_SEGMENTS;
//            float xPos = cos(xSegment * 2.0f * PI) * sin(ySegment * PI);
//            float yPos = cos(ySegment * PI);
//            float zPos = sin(xSegment * 2.0f * PI) * sin(ySegment * PI);
//            // 位置
//            starVertices.push_back(xPos);
//            starVertices.push_back(yPos);
//            starVertices.push_back(zPos);
//            // 法线
//            starVertices.push_back(xPos);
//            starVertices.push_back(yPos);
//            starVertices.push_back(zPos);
//            // 纹理坐标
//            starVertices.push_back(xSegment);
//            starVertices.push_back(ySegment);
//        }
//    }
//
//    // 索引生成
//    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
//    {
//        for (unsigned int x = 0; x < X_SEGMENTS; ++x)
//        {
//            starIndices.push_back(y * (X_SEGMENTS + 1) + x);
//            starIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
//            starIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
//            starIndices.push_back(y * (X_SEGMENTS + 1) + x);
//            starIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
//            starIndices.push_back(y * (X_SEGMENTS + 1) + x + 1);
//        }
//    }
//
//    // 恒星 VAO, VBO, EBO
//    unsigned int starVAO, starVBO, starEBO;
//    glGenVertexArrays(1, &starVAO);
//    glGenBuffers(1, &starVBO);
//    glGenBuffers(1, &starEBO);
//    glBindVertexArray(starVAO);
//    glBindBuffer(GL_ARRAY_BUFFER, starVBO);
//    glBufferData(GL_ARRAY_BUFFER, starVertices.size() * sizeof(float), &starVertices[0], GL_STATIC_DRAW);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, starEBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, starIndices.size() * sizeof(unsigned int), &starIndices[0], GL_STATIC_DRAW);
//    // 位置属性
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
//    // 法线属性
//    glEnableVertexAttribArray(1);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
//    // 纹理坐标属性
//    glEnableVertexAttribArray(2);
//    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
//
//
//    // 日冕公告板顶点数据
//    unsigned int coronaQuadVAO, coronaQuadVBO;
//    float coronaQuadVertices[] = {
//        // positions   // texCoords
//        /*-1.0f,  1.0f, 0.0f, 1.0f,
//        -1.0f, -1.0f, 0.0f, 0.0f,
//         1.0f, -1.0f, 1.0f, 0.0f,
//        -1.0f,  1.0f, 0.0f, 1.0f,
//         1.0f, -1.0f, 1.0f, 0.0f,
//         1.0f,  1.0f, 1.0f, 1.0f*/
//         // positions        // texcoords
//        -2.0f, -2.0f, 0.0f,  0.0f, 0.0f,
//         2.0f, -2.0f, 0.0f,  1.0f, 0.0f,
//         2.0f,  2.0f, 0.0f,  1.0f, 1.0f,
//        -2.0f,  2.0f, 0.0f,  0.0f, 1.0f
//    };
//    glGenVertexArrays(1, &coronaQuadVAO);
//    glGenBuffers(1, &coronaQuadVBO);
//    glBindVertexArray(coronaQuadVAO);
//    glBindBuffer(GL_ARRAY_BUFFER, coronaQuadVBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(coronaQuadVertices), coronaQuadVertices, GL_STATIC_DRAW);
//
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
//
//    glEnableVertexAttribArray(2);
//    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
//
//
//    // =============================================
//    // 帧缓冲四边形
//    unsigned int quadVAO, quadVBO;
//    float quadVertices[] = {
//        // 位置(x,y)      纹理坐标(u,v)
//        -1.0f,  1.0f,     0.0f, 1.0f,
//        -1.0f, -1.0f,     0.0f, 0.0f,
//         1.0f, -1.0f,     1.0f, 0.0f,
//        -1.0f,  1.0f,     0.0f, 1.0f,
//         1.0f, -1.0f,     1.0f, 0.0f,
//         1.0f,  1.0f,     1.0f, 1.0f
//    };
//
//    glGenVertexArrays(1, &quadVAO);
//    glGenBuffers(1, &quadVBO);
//    glBindVertexArray(quadVAO);
//    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(1);
//    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
//	glBindVertexArray(0);
//
//    // ==============================================
//    // 天空盒顶点数据
//    float skyboxVertices[] = {
//        // positions          
//        -1.0f,  1.0f, -1.0f,
//        -1.0f, -1.0f, -1.0f,
//         1.0f, -1.0f, -1.0f,
//         1.0f, -1.0f, -1.0f,
//         1.0f,  1.0f, -1.0f,
//        -1.0f,  1.0f, -1.0f,
//
//        -1.0f, -1.0f,  1.0f,
//        -1.0f, -1.0f, -1.0f,
//        -1.0f,  1.0f, -1.0f,
//        -1.0f,  1.0f, -1.0f,
//        -1.0f,  1.0f,  1.0f,
//        -1.0f, -1.0f,  1.0f,
//
//         1.0f, -1.0f, -1.0f,
//         1.0f, -1.0f,  1.0f,
//         1.0f,  1.0f,  1.0f,
//         1.0f,  1.0f,  1.0f,
//         1.0f,  1.0f, -1.0f,
//         1.0f, -1.0f, -1.0f,
//
//        -1.0f, -1.0f,  1.0f,
//        -1.0f,  1.0f,  1.0f,
//         1.0f,  1.0f,  1.0f,
//         1.0f,  1.0f,  1.0f,
//         1.0f, -1.0f,  1.0f,
//        -1.0f, -1.0f,  1.0f,
//
//        -1.0f,  1.0f, -1.0f,
//         1.0f,  1.0f, -1.0f,
//         1.0f,  1.0f,  1.0f,
//         1.0f,  1.0f,  1.0f,
//        -1.0f,  1.0f,  1.0f,
//        -1.0f,  1.0f, -1.0f,
//
//        -1.0f, -1.0f, -1.0f,
//        -1.0f, -1.0f,  1.0f,
//         1.0f, -1.0f, -1.0f,
//         1.0f, -1.0f, -1.0f,
//        -1.0f, -1.0f,  1.0f,
//         1.0f, -1.0f,  1.0f
//    };
//    // spacebox VAO, VBO
//    unsigned int skyboxVAO, skyboxVBO;
//    glGenVertexArrays(1, &skyboxVAO);
//    glGenBuffers(1, &skyboxVBO);
//    glBindVertexArray(skyboxVAO);
//    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//
//
//    // 加载天空纹理
//    // NightSky
//    //std::vector<std::string> face
//    //{
//    //    ("res/texture/NightSky1024/px.png"),
//    //    ("res/texture/NightSky1024/nx.png"),
//    //    ("res/texture/NightSky1024/py.png"),
//    //    ("res/texture/NightSky1024/ny.png"),
//    //    ("res/texture/NightSky1024/pz.png"),
//    //    ("res/texture/NightSky1024/nz.png")
//    //};
//
//    // SpaceFox
//    std::vector<std::string> face
//    {
//        ("res/texture/SpaceFox1024/px.png"),
//        ("res/texture/SpaceFox1024/nx.png"),
//        ("res/texture/SpaceFox1024/py.png"),
//        ("res/texture/SpaceFox1024/ny.png"),
//        ("res/texture/SpaceFox1024/pz.png"),
//        ("res/texture/SpaceFox1024/nz.png")
//    };
//
//
//    unsigned int cubemapTexture = loadCubemap(face);
//
//    // 设置恒星，星球位置和大小
//    glm::vec3 pointSunPositions = glm::vec3(-50.0f, 50.0f, -600.0f);
//    glm::vec3 SunScale = glm::vec3(120.0f);
//    glm::vec3 planetPosition = glm::vec3(0.0f, -3.0f, 0.0f);
//    glm::vec3 planetScale = glm::vec3(4.0f);
//
//    // 激活着色器纹理单元
//    planetshader.use();
//    planetshader.setInt("material.diffuse", 0); // 告诉着色器每个采样器属于哪个纹理单元(只需要设置一次)
//    planetshader.setInt("material.specular", 1);
//
//
//    // 主循环
//    while (!glfwWindowShouldClose(window))
//    {
//        // 计算帧时间
//        float currentFrame = static_cast<float>(glfwGetTime());
//        deltaTime = currentFrame - lastFrame;
//        lastFrame = currentFrame;
//
//        // 输入管理
//        processInput(window);
//        // 渲染
//        // ------
//        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//
//		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);  // 绑定到HDR帧缓冲
//        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//
//
//        // 配置变换矩阵
//        int winWidth, winHeight;
//        glfwGetFramebufferSize(window, &winWidth, &winHeight);
//        float aspect = winWidth / (float)winHeight;
//        //glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 2000.0f);
//		glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), aspect, 0.1f, 2000.0f);
//        glm::mat4 view = camera.GetViewMatrix();
//
//        // =================================
//        // ===== 天空盒（背景）=====
//        // =================================
//        glDepthMask(GL_TRUE);    // 写入深度
//        glDepthFunc(GL_LEQUAL);
//        glDisable(GL_BLEND);
//
//        spaceboxShader.use();
//        glm::mat4 skyView = glm::mat4(glm::mat3(camera.GetViewMatrix())); // 移除平移
//        spaceboxShader.setMat4("view", skyView);
//        spaceboxShader.setMat4("projection", projection);
//
//        glBindVertexArray(skyboxVAO);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
//        glDrawArrays(GL_TRIANGLES, 0, 36);
//        glBindVertexArray(0);
//
//        glDepthFunc(GL_LESS);   // 恢复默认深度比较
//        // ————————————————————————————————————————————————————————
//        // ======= 天空盒绘制结束 ========
//
//
//        // ========================================
//        // 恒星渲染部分 - 开始
//        // ========================================
//        float starTime = static_cast<float>(glfwGetTime()); // 恒星旋转时间
//        float starPulse = 1.0f + sin(starTime * 1.5f) * 0.0003f; // 计算脉冲效果
//
//        //  设置恒星颜色和强度
//        //glm::vec3 starColor = glm::vec3(1.0f, 0.8f, 0.6f); // 太白了
//        //glm::vec3 coreColor = glm::vec3(1.0f, 0.9f, 0.7f); // 太白了
//        glm::vec3 starColor = glm::vec3(1.0f, 0.65f, 0.3f); // 更暖的颜色
//        glm::vec3 coreColor = glm::vec3(1.0f, 0.75f, 0.4f); // 更暖的核心颜色
//
//        float starIntensity = 3.0f + sin(starTime * 0.7f) * 0.3f; // 计算恒星亮度变化
//
//        // 先保存当前的深度状态和混合状态，以便后续恢复
//        GLboolean depthEnabled;
//        glGetBooleanv(GL_DEPTH_TEST, &depthEnabled);
//        GLboolean blendEnabled;
//        glGetBooleanv(GL_BLEND, &blendEnabled);
//        GLint depthFunc;
//        glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
//        GLboolean depthMask;
//        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
//
//        // 重置深度状态，确保每层都独立
//        glEnable(GL_DEPTH_TEST);
//
//        // 1. 先绘制核心层（最小，最亮）
//        glDepthMask(GL_TRUE); // 核心层深度写入
//        glDisable(GL_BLEND); // 核心不透明，不需要混合
//        glDepthFunc(GL_LESS); // 默认深度函数
//        glDisable(GL_BLEND); // 核心不透明，不需要混合
//
//        float starPulse2 = starPulse * 0.9f; // 核心脉冲幅度更小一些
//        sunCoreShader.use();
//        glm::mat4 coreModel = glm::mat4(1.0f);
//        coreModel = glm::translate(coreModel, pointSunPositions);
//        coreModel = glm::rotate(coreModel, starTime * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
//        coreModel = glm::scale(coreModel, SunScale * 1.0f * starPulse2);
//
//        sunCoreShader.setMat4("model", coreModel);
//        sunCoreShader.setMat4("view", view);
//        sunCoreShader.setMat4("projection", projection);
//        sunCoreShader.setFloat("time", starTime);
//        sunCoreShader.setVec3("starColor", starColor);
//        //sunCoreShader.setVec3("coreColor", glm::vec3(1.0f, 0.9f, 0.7f));
//        sunCoreShader.setVec3("coreColor", coreColor);
//        sunCoreShader.setVec3("viewPos", camera.Position);
//        sunCoreShader.setFloat("intensity", starIntensity);
//
//        glBindVertexArray(starVAO);
//        glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);
//        glBindVertexArray(0);
//        //—————————————————————————————————————————————————————————————————————
//
//
//        //—————————————————————————————————————————————————————————————————————
//        // 2. 绘制日冕层（中等大小）
//        //  关键：重置深度状态，使用不同的深度函数
//        glDepthMask(GL_FALSE); //  日冕层不写入深度
//        glDepthFunc(GL_LEQUAL); //  允许深度值相等或更大的片段通过
//        glEnable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 使用加法混合让日冕更亮
//
//        // 喷涌日冕
//        sunCoronaShader.use();
//        glm::vec3 dirToCamera = camera.Position - pointSunPositions;
//        glm::mat4 coronaModel = glm::mat4(1.0f);
//        coronaModel = glm::translate(coronaModel, pointSunPositions);
//        glm::mat4 rotate = glm::inverse(glm::lookAt(glm::vec3(0.f), dirToCamera, camera.WorldUp));
//        coronaModel = coronaModel * rotate;
//        
//        coronaModel = glm::scale(coronaModel, SunScale * 2.0f * starPulse); // 日冕层比核心大
//
//        sunCoronaShader.setMat4("model", coronaModel);
//        sunCoronaShader.setMat4("view", view);
//        sunCoronaShader.setMat4("projection", projection);
//        sunCoronaShader.setFloat("time", starTime * 0.1f);
//		sunCoronaShader.setVec3("sunCenter", pointSunPositions);
//        sunCoronaShader.setVec3("coronaColor", starColor);
//        //sunCoronaShader.setVec3("viewPos", camera.Position);
//        sunCoronaShader.setFloat("coronaIntensity", 1.8f);
//        sunCoronaShader.setFloat("scale", SunScale.x * 1.7f); // 204.0f
//
//        glBindVertexArray(coronaQuadVAO);
//        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);   // 用4个顶点绘制四边形
//        glBindVertexArray(0);
//
//        // 环绕日冕
//        CoreCoronaShader.use();
//        coronaModel = glm::mat4(1.0f);
//        coronaModel = glm::translate(coronaModel, pointSunPositions);
//        coronaModel = glm::rotate(coronaModel, starTime * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
//        coronaModel = glm::scale(coronaModel, SunScale * 1.7f * starPulse); // 日冕层比核心大
//
//        CoreCoronaShader.setMat4("model", coronaModel);
//        CoreCoronaShader.setMat4("view", view);
//        CoreCoronaShader.setMat4("projection", projection);
//        CoreCoronaShader.setFloat("time", starTime);
//		CoreCoronaShader.setVec3("sunCenter", pointSunPositions);
//        CoreCoronaShader.setVec3("coronaColor", starColor);
//        //CoreCoronaShader.setVec3("viewPos", camera.Position);
//        CoreCoronaShader.setFloat("coronaIntensity", 1.8f);
//        CoreCoronaShader.setFloat("coronaRadius", SunScale.x * 1.7f); // 204.0f
//
//        glBindVertexArray(starVAO);
//        glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);
//        glBindVertexArray(0);
//
//
//        //—————————————————————————————————————————————————————————————————————
//        // 3. 最后绘制辉光层（最大，最透明）
//        // 修改深度状态，混合状态
//        glDepthMask(GL_FALSE); // 不写入深度
//        glDepthFunc(GL_LEQUAL); // 等于或更大深度通过
//        glEnable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // 使用加法混合！
//
//        sunGlowShader.use();
//        glm::mat4 glowModel = glm::mat4(1.0f);
//        glowModel = glm::translate(glowModel, pointSunPositions);
//        glowModel = glm::rotate(glowModel, starTime * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
//        glowModel = glm::scale(glowModel, SunScale * 1.7f * starPulse);
//
//        sunGlowShader.setMat4("model", glowModel);
//        sunGlowShader.setMat4("view", view);
//        sunGlowShader.setMat4("projection", projection);
//        sunGlowShader.setFloat("time", starTime);
//        sunGlowShader.setVec3("starColor", starColor);
//        sunGlowShader.setVec3("viewPos", camera.Position);
//        sunGlowShader.setFloat("glowIntensity", 0.8f);
//
//        glBindVertexArray(starVAO);
//        glDrawElements(GL_TRIANGLES, starIndices.size(), GL_UNSIGNED_INT, 0);
//        glBindVertexArray(0);
//        //—————————————————————————————————————————————————————————————————————
//        
//        // ========================================
//        // 恒星渲染部分 - 结束
//        // ========================================
//
//        // 恢复原始状态
//        if (!depthEnabled) glDisable(GL_DEPTH_TEST);
//        else glEnable(GL_DEPTH_TEST);
//        glDepthMask(depthMask ? GL_TRUE : GL_FALSE);
//        glDepthFunc(depthFunc);
//        if (blendEnabled) glEnable(GL_BLEND);
//        else glDisable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 恢复默认混合函数
//
//
//        // ========================================
//        // 行星渲染
//        // ========================================
//        //—————————————————————————————————————————————————————————————————————
//        // 行星着色器
//        planetshader.use();
//        planetshader.setVec3("viewPos", camera.Position);
//        planetshader.setFloat("material.shininess", 32.0f);
//        // 定向光
//        planetshader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
//        planetshader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
//        planetshader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
//        planetshader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
//        // 点光源 1
//        planetshader.setVec3("pointLights[0].position", pointSunPositions);
//        planetshader.setVec3("pointLights[0].ambient", 1.0f, 1.0f, 0.8f);
//        planetshader.setVec3("pointLights[0].diffuse", 200.0f, 200.0f, 160.0f);   // 增大
//        planetshader.setVec3("pointLights[0].specular", 120.0f, 120.0f, 100.0f);  // 增大
//        planetshader.setFloat("pointLights[0].constant", 1.0f);
//        planetshader.setFloat("pointLights[0].linear", 0.0002f);        // 原来是0.09，减小
//        planetshader.setFloat("pointLights[0].quadratic", 0.000005f);    // 原来是0.032，减小
//
//        planetshader.setMat4("projection", projection);
//        planetshader.setMat4("view", view);
//
//        // 绘制行星
//        glm::mat4 model = glm::mat4(1.0f);
//        model = glm::translate(model, planetPosition);
//        model = glm::scale(model, planetScale);
//        planetshader.setMat4("model", model);
//        planet.Draw(planetshader);
//
//
//        // 设置小行星着色器并绘制
//        asteroidShader.use();
//        asteroidShader.setMat4("projection", projection);
//        asteroidShader.setMat4("view", view);
//
//        asteroidShader.use();
//        asteroidShader.setInt("material.diffuse", 0);
//        asteroidShader.setInt("material.specular", 1);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, rock.textures_loaded[0].id);
//        // 设置小行星着色器并绘制
//        asteroidShader.use();
//        asteroidShader.setVec3("viewPos", camera.Position);
//        asteroidShader.setFloat("material.shininess", 32.0f);
//        // 定向光
//        asteroidShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
//        asteroidShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
//        asteroidShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
//        asteroidShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
//        // 点光源 1
//        asteroidShader.setVec3("pointLights[0].position", pointSunPositions);
//        asteroidShader.setVec3("pointLights[0].ambient", 1.0f, 1.0f, 0.8f);
//        asteroidShader.setVec3("pointLights[0].diffuse", 200.0f, 200.0f, 160.0f);   // 增大
//        asteroidShader.setVec3("pointLights[0].specular", 120.0f, 120.0f, 100.0f);  // 增大
//        asteroidShader.setFloat("pointLights[0].constant", 1.0f);
//        asteroidShader.setFloat("pointLights[0].linear", 0.0002f);        // 原来是0.09，减小
//        asteroidShader.setFloat("pointLights[0].quadratic", 0.000005f);    // 原来是0.032，减小
//
//        // 绘制小行星
//        for (unsigned int i = 0; i < rock.meshes.size(); i++)
//        {
//
//            glBindVertexArray(rock.meshes[i].VAO);
//            glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(rock.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, amount);
//            glBindVertexArray(0);
//        }
//
//		//==========================================
//		// 渲染到屏幕
//        glDisable(GL_DEPTH_TEST);
//        glDisable(GL_BLEND);
//
//		// 1. 亮度提取：从hdrColorBuffer中提取亮度信息到blurFBO1
//		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO1);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		brightPassShader.use();
//        glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
//		brightPassShader.setInt("hdrImage", 0);
//		brightPassShader.setFloat("threshold", 1.2f); // 设置亮度阈值
//		glBindVertexArray(quadVAO);
//		glDrawArrays(GL_TRIANGLES, 0, 6);
//
//		// 2. 水平高斯模糊
//		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO2);
//        glClear(GL_COLOR_BUFFER_BIT);
//        blurShader.use();
//		glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, blurColorBuffer1);
//		blurShader.setInt("image", 0);
//        blurShader.setBool("horizontal", true); // 水平模糊
//		glBindVertexArray(quadVAO);
//		glDrawArrays(GL_TRIANGLES, 0, 6);
//
//		// 3. 垂直高斯模糊
//		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO1);
//        glClear(GL_COLOR_BUFFER_BIT);
//		blurShader.use();
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, blurColorBuffer2);
//		blurShader.setBool("horizontal", false); // 垂直模糊
//		glBindVertexArray(quadVAO);
//		glDrawArrays(GL_TRIANGLES, 0, 6);
//		// ------- 合成到屏幕 -------
//		glBindFramebuffer(GL_FRAMEBUFFER, 0);   // 回到默认帧缓冲
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        compositeShader.use();
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
//		compositeShader.setInt("sceneTexture", 0);
//		glActiveTexture(GL_TEXTURE1);
//		glBindTexture(GL_TEXTURE_2D, blurColorBuffer1); // 最终模糊结果在 blurFBO1 的颜色附件
//		compositeShader.setInt("bloomTexture", 1);
//		compositeShader.setFloat("exposure", 1.0f); // 曝光值
//		compositeShader.setFloat("bloomStrength", 0.6f); // Bloom强度
//		compositeShader.setVec3("colorTint", glm::vec3(1.0f, 0.95f, 0.6f)); // Bloom颜色
//		glBindVertexArray(quadVAO);
//		glDrawArrays(GL_TRIANGLES, 0, 6);
//
//        // 恢复原始状态
//        if (!depthEnabled) glDisable(GL_DEPTH_TEST);
//        else glEnable(GL_DEPTH_TEST);
//        glDepthMask(depthMask ? GL_TRUE : GL_FALSE);
//        glDepthFunc(depthFunc);
//        if (blendEnabled) glEnable(GL_BLEND);
//        else glDisable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 恢复默认混合函数
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    glDeleteVertexArrays(1, &starVAO);
//    glDeleteBuffers(1, &starVBO);
//    glDeleteVertexArrays(1, &skyboxVAO);
//    glDeleteBuffers(1, &skyboxVBO);
//
//    glfwTerminate(); // 清理并关闭GLFW
//    return 0;
//}
//
//
//
//
//// 窗口回调函数
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//    glViewport(0, 0, width, height); // glViewport 用于设置视口大小
//    lastX = width / 2.0f;
//    lastY = height / 2.0f;
//}
//
//// 输入检查函数
//void processInput(GLFWwindow* window)
//{
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, true);
//
//    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
//        camera.ProcessKeyboard(FORWARD, deltaTime);
//    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
//        camera.ProcessKeyboard(BACKWARD, deltaTime);
//    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
//        camera.ProcessKeyboard(LEFT, deltaTime);
//    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
//        camera.ProcessKeyboard(RIGHT, deltaTime);
//
//    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
//		camera.Sensitivity += 0.001f; // 增加鼠标灵敏度
//    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
//        camera.Sensitivity -= 0.001f; // 减少鼠标灵敏度
//    // 限制范围
//    camera.Sensitivity = glm::clamp(camera.Sensitivity, 0.01f, 0.5f);
//
//    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabKeyPressed) 
//    {
//        tabKeyPressed = true;
//        if (cursorLocked) 
//        {
//            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//            cursorLocked = false;
//        }
//        else 
//        {
//            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//            cursorLocked = true;
//            firstMouse = true;
//        }
//    }
//    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE)
//    {
//        tabKeyPressed = false;
//    }
//}
//
//void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
//{
//    float xpos = static_cast<float>(xposIn);
//    float ypos = static_cast<float>(yposIn);
//
//    if (firstMouse)
//    {
//        lastX = xpos;
//        lastY = ypos;
//        firstMouse = false;
//    }
//
//    float xoffset = xpos - lastX;
//    float yoffset = lastY - ypos;
//
//    lastX = xpos;
//    lastY = ypos;
//
//    camera.ProcessMouseMovement(xoffset, yoffset);
//}
//
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
//{
//    camera.ProcessMouseScroll(static_cast<float>(yoffset));
//}
//
//unsigned int loadTexture(char const* path)
//{
//    unsigned int textureID;
//    glGenTextures(1, &textureID);
//
//    int width, height, nrComponents;
//    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
//    if (data)
//    {
//        GLenum format;
//        if (nrComponents == 1)
//            format = GL_RED;
//        else if (nrComponents == 3)
//            format = GL_RGB;
//        else if (nrComponents == 4)
//            format = GL_RGBA;
//
//        glBindTexture(GL_TEXTURE_2D, textureID);
//        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//        //glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
//        glGenerateMipmap(GL_TEXTURE_2D);
//
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//        stbi_image_free(data);
//    }
//    else
//    {
//        std::cout << "Texture failed to load at path: " << path << std::endl;
//        stbi_image_free(data);
//    }
//
//    return textureID;
//}
//
//// 加载立方体贴图
//unsigned int loadCubemap(std::vector<std::string> faces)
//{
//    unsigned int textureID;
//    glGenTextures(1, &textureID);
//    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
//
//    // Ensure cubemap faces are not flipped vertically
//    stbi_set_flip_vertically_on_load(false);
//
//    int width, height, nrChannels;
//    for (unsigned int i = 0; i < faces.size(); i++)
//    {
//        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
//        if (data)
//        {
//            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//            stbi_image_free(data);
//        }
//        else
//        {
//            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
//            stbi_image_free(data);
//        }
//        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//    }
//
//    return textureID;
//}
//
//// 从文件加载纹理
//unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
//{
//    string filename = string(path);
//    filename = directory + '/' + filename;
//
//    unsigned int textureID;
//    glGenTextures(1, &textureID);
//
//    int width, height, nrComponents;
//    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
//    if (data)
//    {
//        GLenum format;
//        if (nrComponents == 1)
//            format = GL_RED;
//        else if (nrComponents == 3)
//            format = GL_RGB;
//        else if (nrComponents == 4)
//            format = GL_RGBA;
//
//        glBindTexture(GL_TEXTURE_2D, textureID);
//        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//        glGenerateMipmap(GL_TEXTURE_2D);
//
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//        stbi_image_free(data);
//    }
//
//    return textureID;
//}
//
//
////unsigned int hdrFBO, blurFBO1, blurFBO2;
////unsigned int hdrColorBuffer, blurColorBuffer1, blurColorBuffer2;
////unsigned int hdrDepthRBO;
//void setupFramebuffers(int width, int height)
//{
//    glGenFramebuffers(1, &hdrFBO);
//    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
//    glGenTextures(1, &hdrColorBuffer);
//    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorBuffer, 0);
//
//    glGenRenderbuffers(1, &hdrDepthRBO);
//    glBindRenderbuffer(GL_RENDERBUFFER, hdrDepthRBO);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrDepthRBO);
//    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//        std::cout << "HDR FBO incomplete!" << std::endl;
//
//    glGenFramebuffers(1, &blurFBO1);
//    glBindFramebuffer(GL_FRAMEBUFFER, blurFBO1);
//    glGenTextures(1, &blurColorBuffer1);
//    glBindTexture(GL_TEXTURE_2D, blurColorBuffer1);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurColorBuffer1, 0);
//    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//        std::cout << "Blur FBO1 incomplote!" << std::endl;
//
//    glGenFramebuffers(1, &blurFBO2);
//    glBindFramebuffer(GL_FRAMEBUFFER, blurFBO2);
//    glGenTextures(1, &blurColorBuffer2);
//    glBindTexture(GL_TEXTURE_2D, blurColorBuffer2);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurColorBuffer2, 0);
//    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//        std::cout << "Blur FBO2 incomplete!" << std::endl;
//
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}
//
//
