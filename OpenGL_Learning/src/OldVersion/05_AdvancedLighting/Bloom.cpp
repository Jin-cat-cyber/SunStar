//#include <glad/glad.h>
//#include <glfw3.h>
//#include <assimp/config.h>
//#include <assimp/revision.h>
//
//#include "Shader.h" // 包含自定义着色器类
//
//
//#include <iostream>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//
//#include "camera.h"
//#include "Model.h"
//
//
//#ifdef BLOOM
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>
//
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height); // 窗口大小回调函数
//void processInput(GLFWwindow* window); // 输入检查函数
//void mouse_callback(GLFWwindow* window, double xpos, double ypos); // 鼠标 移动 回调函数
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // 鼠标 滚轮 回调函数
//unsigned int loadTexture(char const* path); // 纹理加载函数
//
//// 渲染函数
//void renderScene(const Shader& shader); // 场景渲染函数
//void renderCube();
//void renderQuad();
//
//// 窗口参数
//const unsigned int SCR_WIDTH = 960; // 窗口宽度
//const unsigned int SCR_HEIGHT = 600; // 窗口高度
//int windowwidth = SCR_WIDTH;
//int windowheight = SCR_HEIGHT;
//
//// 键盘控制
//bool hdr = true; // 是否开启HDR
//bool hdrKeyPressed = false; // 是否按下了空格键
//float exposure = 1.0f; // 曝光值
//bool shadows = true; // 是否开启阴影
//bool shadowsKeyPressed = false; // 是否按下了空格键
//
//// 摄像机相关
//Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
//float lastX = SCR_WIDTH / 2.0F;
//float lastY = SCR_HEIGHT / 2.0F;
//bool firstMouse = true;
//bool cursorLocked = true;
//bool tabKeyPressed = false;   // 用于检测 TAB 键的上升沿
//bool bloom = true;
//bool bloomKeyPressed;
//
//// 设置帧数渲染位置
//float deltaTime = 0.0f;	// 当前帧与上一帧的时间差
//float lastFrame = 0.0f; // 上一帧的时间
//
//// 设置平面VAO
//unsigned int planeVAO;
//
//// HDR帧缓冲
////unsigned int hdrFBO = 0;
////unsigned int colorBuffer = 0;
////unsigned int rboDepth = 0;
//
//// HDR帧缓冲处理函数
////void HDRSet();
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
//    //glEnable(GL_MULTISAMPLE); // 启用多重采样抗锯齿
//    //glEnable(GL_CULL_FACE); // 启用面剔除
//
//    // 创建着色器对象    
//    /*Shader shader("res/shader/HDR/hdr_lighting_ver.shader", "res/shader/HDR/hdr_lighting_frag.shader");
//    Shader hdrShader("res/shader/HDR/hdr_ver.shader", "res/shader/HDR/hdr_frag.shader");*/
//
//    Shader shader("res/shader/Bloom/bloom_ver.shader", "res/shader/Bloom/bloom_frag.shader");
//    Shader shaderLight("res/shader/Bloom/bloom_ver.shader", "res/shader/Bloom/bloom_frag.shader");
//    Shader shaderBlur("res/shader/Bloom/blur_ver.shader", "res/shader/Bloom/blur_frag.shader");
//    Shader shaderBloomFinal("res/shader/Bloom/bloom_final_ver.shader", "res/shader/Bloom/bloom_final_frag.shader");
//    
//    // 加载贴图
//    unsigned int woodTexture = loadTexture("res/texture/wood.png");
//    unsigned int containerTexture = loadTexture("res/texture/container2.png");
//
//    unsigned int hdrFBO;
//    glGenFramebuffers(1, &hdrFBO);
//    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
//    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
//    unsigned int colorBuffers[2];
//    glGenTextures(2, colorBuffers);
//    for (unsigned int i = 0; i < 2; i++)
//    {
//        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        // attach texture to framebuffer
//        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
//    }
//    // create and attach depth buffer (renderbuffer)
//    unsigned int rboDepth;
//    glGenRenderbuffers(1, &rboDepth);
//    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
//    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
//    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
//    glDrawBuffers(2, attachments);
//    // finally check if framebuffer is complete
//    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//        std::cout << "Framebuffer not complete!" << std::endl;
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//    // ping-pong-framebuffer for blurring
//    unsigned int pingpongFBO[2];
//    unsigned int pingpongColorbuffers[2];
//    glGenFramebuffers(2, pingpongFBO);
//    glGenTextures(2, pingpongColorbuffers);
//    for (unsigned int i = 0; i < 2; i++)
//    {
//        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
//        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
//        // also check if framebuffers are complete (no need for depth buffer)
//        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//            std::cout << "Framebuffer not complete!" << std::endl;
//    }
//
//    // lighting info
//    // -------------
//    // positions
//    std::vector<glm::vec3> lightPositions;
//    lightPositions.push_back(glm::vec3(0.0f, 0.5f, 1.5f));
//    lightPositions.push_back(glm::vec3(-4.0f, 0.5f, -3.0f));
//    lightPositions.push_back(glm::vec3(3.0f, 0.5f, 1.0f));
//    lightPositions.push_back(glm::vec3(-.8f, 2.4f, -1.0f));
//    // colors
//    std::vector<glm::vec3> lightColors;
//    lightColors.push_back(glm::vec3(5.0f, 5.0f, 5.0f));
//    lightColors.push_back(glm::vec3(10.0f, 0.0f, 0.0f));
//    lightColors.push_back(glm::vec3(0.0f, 0.0f, 15.0f));
//    lightColors.push_back(glm::vec3(0.0f, 5.0f, 0.0f));
//
//
//    // shader configuration
//    // --------------------
//    shader.use();
//    shader.setInt("diffuseTexture", 0);
//    shaderBlur.use();
//    shaderBlur.setInt("image", 0);
//    shaderBloomFinal.use();
//    shaderBloomFinal.setInt("scene", 0);
//    shaderBloomFinal.setInt("bloomBlur", 1);
//
//    // 光源位置
//    // -------------
//    //glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
//
//    // render loop
//    // -----------
//    // 主循环
//    while (!glfwWindowShouldClose(window))
//    {
//        // 时钟逻辑
//        // --------------------
//        float currentFrame = static_cast<float>(glfwGetTime());
//        deltaTime = currentFrame - lastFrame;
//        lastFrame = currentFrame;
//
//        // 输入
//        // -----
//        processInput(window);
//
//
//        // 渲染
//        // ------
//        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//        // 1. render scene into floating point framebuffer
//       // -----------------------------------------------
//        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
//        glm::mat4 view = camera.GetViewMatrix();
//        glm::mat4 model = glm::mat4(1.0f);
//        shader.use();
//        shader.setMat4("projection", projection);
//        shader.setMat4("view", view);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, woodTexture);
//        // set lighting uniforms
//        for (unsigned int i = 0; i < lightPositions.size(); i++)
//        {
//            shader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
//            shader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
//        }
//        shader.setVec3("viewPos", camera.Position);
//        // create one large cube that acts as the floor
//        model = glm::mat4(1.0f);
//        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0));
//        model = glm::scale(model, glm::vec3(12.5f, 0.5f, 12.5f));
//        shader.setMat4("model", model);
//        renderCube();
//        // then create multiple cubes as the scenery
//        glBindTexture(GL_TEXTURE_2D, containerTexture);
//        model = glm::mat4(1.0f);
//        model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
//        model = glm::scale(model, glm::vec3(0.5f));
//        shader.setMat4("model", model);
//        renderCube();
//
//        model = glm::mat4(1.0f);
//        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
//        model = glm::scale(model, glm::vec3(0.5f));
//        shader.setMat4("model", model);
//        renderCube();
//
//        model = glm::mat4(1.0f);
//        model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 2.0));
//        model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
//        shader.setMat4("model", model);
//        renderCube();
//
//        model = glm::mat4(1.0f);
//        model = glm::translate(model, glm::vec3(0.0f, 2.7f, 4.0));
//        model = glm::rotate(model, glm::radians(23.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
//        model = glm::scale(model, glm::vec3(1.25));
//        shader.setMat4("model", model);
//        renderCube();
//
//        model = glm::mat4(1.0f);
//        model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -3.0));
//        model = glm::rotate(model, glm::radians(124.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
//        shader.setMat4("model", model);
//        renderCube();
//
//        model = glm::mat4(1.0f);
//        model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
//        model = glm::scale(model, glm::vec3(0.5f));
//        shader.setMat4("model", model);
//        renderCube();
//
//        // finally show all the light sources as bright cubes
//        shaderLight.use();
//        shaderLight.setMat4("projection", projection);
//        shaderLight.setMat4("view", view);
//
//        for (unsigned int i = 0; i < lightPositions.size(); i++)
//        {
//            model = glm::mat4(1.0f);
//            model = glm::translate(model, glm::vec3(lightPositions[i]));
//            model = glm::scale(model, glm::vec3(0.25f));
//            shaderLight.setMat4("model", model);
//            shaderLight.setVec3("lightColor", lightColors[i]);
//            renderCube();
//        }
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//        // 2. blur bright fragments with two-pass Gaussian Blur 
//        // --------------------------------------------------
//        bool horizontal = true, first_iteration = true;
//        unsigned int amount = 10;
//        shaderBlur.use();
//        for (unsigned int i = 0; i < amount; i++)
//        {
//            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
//            shaderBlur.setInt("horizontal", horizontal);
//            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
//            renderQuad();
//            horizontal = !horizontal;
//            if (first_iteration)
//                first_iteration = false;
//        }
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//        // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
//        // --------------------------------------------------------------------------------------------------------------------------
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        shaderBloomFinal.use();
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
//        shaderBloomFinal.setInt("bloom", bloom);
//        shaderBloomFinal.setFloat("exposure", exposure);
//        renderQuad();
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    glfwTerminate(); // 清理并关闭GLFW
//    return 0;
//}
//
//
//// 渲染3D场景：地面和立方体
//// --------------------
//void renderScene(const Shader& shader)
//{
//    // room cube
//    glm::mat4 model = glm::mat4(1.0f);
//    model = glm::scale(model, glm::vec3(10.0f));
//    shader.setMat4("model", model);
//    glDisable(GL_CULL_FACE); // 禁用面剔除，以便从内部看到立方体
//    shader.setInt("reverse_normals", 1); // 反转法线，以便立方体内部正确计算光照
//    renderCube();
//    shader.setInt("reverse_normals", 0); // 恢复默认的法线设置
//    glEnable(GL_CULL_FACE); // 恢复默认的面剔除设置
//
//    // cubes
//    model = glm::mat4(1.0f);
//    model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
//    model = glm::scale(model, glm::vec3(0.5f));
//    shader.setMat4("model", model);
//    renderCube();
//    model = glm::mat4(1.0f);
//    model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0));
//    model = glm::scale(model, glm::vec3(0.75f));
//    shader.setMat4("model", model);
//    renderCube();
//    model = glm::mat4(1.0f);
//    model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0));
//    model = glm::scale(model, glm::vec3(0.5f));
//    shader.setMat4("model", model);
//    renderCube();
//    model = glm::mat4(1.0f);
//    model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5));
//    model = glm::scale(model, glm::vec3(0.5f));
//    shader.setMat4("model", model);
//    renderCube();
//    model = glm::mat4(1.0f);
//    model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
//    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
//    model = glm::scale(model, glm::vec3(0.75f));
//    shader.setMat4("model", model);
//    renderCube();
//}
//
//// renderCube() 渲染一个位于 NDC 中的 1x1 3D 立方体。
//// -----------------------------------------
//unsigned int cubeVAO = 0;
//unsigned int cubeVBO = 0;
//void renderCube()
//{
//    if (cubeVAO == 0)
//    {
//        float vertices[] = {
//            // back face
//            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
//             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
//             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
//             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
//            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
//            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
//            // front face
//            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
//             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
//             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
//             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
//            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
//            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
//            // left face
//            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
//            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
//            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
//            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
//            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
//            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
//            // right face
//             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
//             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
//             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
//             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
//             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
//             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
//             // bottom face
//             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
//              1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
//              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
//              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
//             -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
//             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
//             // top face
//             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
//              1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
//              1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
//              1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
//             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
//             -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
//        };
//        glGenVertexArrays(1, &cubeVAO);
//        glGenBuffers(1, &cubeVBO);
//        // 填充缓冲区
//        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
//        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//        // 链接顶点属性
//        glBindVertexArray(cubeVAO);
//        glEnableVertexAttribArray(0);
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
//        glEnableVertexAttribArray(1);
//        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
//        glEnableVertexAttribArray(2);
//        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        glBindVertexArray(0);
//    }
//
//    // render Cube
//    glBindVertexArray(cubeVAO);
//    glDrawArrays(GL_TRIANGLES, 0, 36);
//    glBindVertexArray(0);
//}
//
//// renderQuad() 在 NDC 中渲染一个 1x1 的 XY 四边形。
//// ---------------------------------------------
//unsigned int quadVAO = 0;
//unsigned int quadVBO;
//void renderQuad()
//{
//    if (quadVAO == 0)
//    {
//        float quadVertices[] = {
//            // positions        // texture Coords
//            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
//            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
//             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
//             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
//        };
//        // 设置 plane VAO
//        glGenVertexArrays(1, &quadVAO);
//        glGenBuffers(1, &quadVBO);
//        glBindVertexArray(quadVAO);
//        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
//        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
//        glEnableVertexAttribArray(0);
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
//        glEnableVertexAttribArray(1);
//        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
//    }
//    glBindVertexArray(quadVAO);
//    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//    glBindVertexArray(0);
//}
//
////// HDR 帧缓冲设置函数
////void HDRSet()
////{
////    //unsigned int hdrFBO;
////    glGenFramebuffers(1, &hdrFBO);
////
////    //unsigned int colorBuffer;
////    glGenTextures(1, &colorBuffer);
////    glBindTexture(GL_TEXTURE_2D, colorBuffer);
////    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowwidth, windowheight, 0, GL_RGBA, GL_FLOAT, NULL);
////    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
////    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
////
////    //unsigned int rboDepth;
////    glGenRenderbuffers(1, &rboDepth);
////    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
////    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowwidth, windowheight);
////
////    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
////    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorBuffer, 0);
////    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
////    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
////    {
////        std::cout << "Framebuffer not complete!" << std::endl;
////    }
////    glBindFramebuffer(GL_FRAMEBUFFER, 0);
////}
//
//
//
//// 窗口回调函数
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//    //windowwidth = width;
//    //windowheight = height;
//    //glViewport(0, 0, width, height); // glViewport 用于设置视口大小
//    //lastX = width / 2.0f;
//    //lastY = height / 2.0f;
//
//    if (width == 0 || height == 0) return; // 避免最小化时异常
//
//    windowwidth = width;
//    windowheight = height;
//    glViewport(0, 0, width, height);
//    lastX = width / 2.0f;
//    lastY = height / 2.0f;
//
//    //// 重新分配 HDR 颜色纹理和深度缓冲（如果已创建）
//    //if (colorBuffer != 0)
//    //{
//    //    glBindTexture(GL_TEXTURE_2D, colorBuffer);
//    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowwidth, windowheight, 0, GL_RGBA, GL_FLOAT, NULL);
//    //}
//    //if (rboDepth != 0)
//    //{
//    //    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
//    //    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowwidth, windowheight);
//    //}
//    // 如果你在回调后需要马上检查 FBO 完整性，可绑定 hdrFBO 并检查，但通常上面足够
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
//
//    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bloomKeyPressed)
//    {
//        bloom = !bloom;
//        bloomKeyPressed = true;
//    }
//    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
//    {
//        bloomKeyPressed = false;
//    }
//
//    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
//    {
//        if (exposure > 0.0f)
//            exposure -= 0.001f;
//        else
//            exposure = 0.0f;
//    }
//    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
//    {
//        exposure += 0.001f;
//    }
//}
//
////void framebuffer_size_callback(GLFWwindow* window, int width, int height)
////{
////    // make sure the viewport matches the new window dimensions; note that width and 
////    // height will be significantly larger than specified on retina displays.
////    glViewport(0, 0, width, height);
////}
//
//// 鼠标移动回调函数
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
//// 鼠标滚轮回调函数
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
//{
//    camera.ProcessMouseScroll(static_cast<float>(yoffset));
//}
//
//// 纹理加载函数
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
//#endif