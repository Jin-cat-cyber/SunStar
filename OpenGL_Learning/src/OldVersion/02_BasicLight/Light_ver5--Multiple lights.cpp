//#include <glad/glad.h>
//#include <glfw3.h>
//#include <assimp/config.h>
//#include <assimp/revision.h>
//
//#include "Shader.h" // 包含自定义着色器类
//
//#include <iostream>
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h> // 用于加载纹理图像
//
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//
//#include "camera.h"
////using namespace std;
//
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height); // 窗口大小回调函数
//void processInput(GLFWwindow* window); // 输入检查函数
//void mouse_callback(GLFWwindow* window, double xpos, double ypos); // 鼠标 移动 回调函数
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // 鼠标 滚轮 回调函数
//unsigned int loadTexture(char const* path); // 纹理加载函数
//
//const unsigned int SCR_WIDTH = 960; // 窗口宽度
//const unsigned int SCR_HEIGHT = 600; // 窗口高度
//
//// 摄像机相关
//Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
//float lastX = SCR_WIDTH / 2.0F;
//float lastY = SCR_HEIGHT / 2.0F;
//bool firstMouse = true;
//
//// 设置帧数渲染位置
//float deltaTime = 0.0f;	// 当前帧与上一帧的时间差
//float lastFrame = 0.0f; // 上一帧的时间
//
//// 设置光源位置
//glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
//
//
//int main()
//{
//    glfwInit(); // 初始化GLFW库
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // 设置OpenGL版本：主版本号
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // 设置OpenGL版本：次版本号
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 使用核心模式
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
//    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//
//
//    // 初始化GLAD，管理OpenGL函数指针，加载所有OpenGL函数指针
//    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//    {
//        std::cout << "Failed to initialize GLAD" << std::endl;
//        return -1;
//    }
//
//    //加载深度缓冲
//    glEnable(GL_DEPTH_TEST);
//
//    // 创建着色器对象
//    Shader lightingShader("res/shader/2.0_ver_color.shader", "res/shader/2.0_frag_color.shader");
//    Shader lightCubeShader("res/shader/ver_light_cube.shader", "res/shader/frag_light_cube.shader");
//
//    // 顶点数据
//    float vertices[] = {
//        // positions          // normals           // texture coords
//        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f,         -1.0f,  0.0f, 0.0f,
//         0.5f, -0.5f, -0.5f,  0.0f,  0.0f,         -1.0f,  1.0f, 0.0f,
//         0.5f,  0.5f, -0.5f,  0.0f,  0.0f,         -1.0f,  1.0f, 1.0f,
//         0.5f,  0.5f, -0.5f,  0.0f,  0.0f,         -1.0f,  1.0f, 1.0f,
//        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f,         -1.0f,  0.0f, 1.0f,
//        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f,         -1.0f,  0.0f, 0.0f,
//
//        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,         1.0f,   0.0f, 0.0f,
//         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,         1.0f,   1.0f, 0.0f,
//         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,         1.0f,   1.0f, 1.0f,
//         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,         1.0f,   1.0f, 1.0f,
//        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,         1.0f,   0.0f, 1.0f,
//        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,         1.0f,   0.0f, 0.0f,
//
//        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,          0.0f,  1.0f, 0.0f,
//        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,          0.0f,  1.0f, 1.0f,
//        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,          0.0f,  0.0f, 1.0f,
//        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,          0.0f,  0.0f, 1.0f,
//        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,          0.0f,  0.0f, 0.0f,
//        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,          0.0f,  1.0f, 0.0f,
//
//         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,          0.0f,  1.0f, 0.0f,
//         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,          0.0f,  1.0f, 1.0f,
//         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,          0.0f,  0.0f, 1.0f,
//         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,          0.0f,  0.0f, 1.0f,
//         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,          0.0f,  0.0f, 0.0f,
//         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,          0.0f,  1.0f, 0.0f,
//
//        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,          0.0f,  0.0f, 1.0f,
//         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,          0.0f,  1.0f, 1.0f,
//         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,          0.0f,  1.0f, 0.0f,
//         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,          0.0f,  1.0f, 0.0f,
//        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,          0.0f,  0.0f, 0.0f,
//        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,          0.0f,  0.0f, 1.0f,
//
//        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,          0.0f,  0.0f, 1.0f,
//         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,          0.0f,  1.0f, 1.0f,
//         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,          0.0f,  1.0f, 0.0f,
//         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,          0.0f,  1.0f, 0.0f,
//        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,          0.0f,  0.0f, 0.0f,
//        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,          0.0f,  0.0f, 1.0f
//    };
//    // 立方体位置
//    glm::vec3 cubePositions[] = {
//    glm::vec3(0.0f,  0.0f,  0.0f),
//    glm::vec3(2.0f,  5.0f, -15.0f),
//    glm::vec3(-1.5f, -2.2f, -2.5f),
//    glm::vec3(-3.8f, -2.0f, -12.3f),
//    glm::vec3(2.4f, -0.4f, -3.5f),
//    glm::vec3(-1.7f,  3.0f, -7.5f),
//    glm::vec3(1.3f, -2.0f, -2.5f),
//    glm::vec3(1.5f,  2.0f, -2.5f),
//    glm::vec3(1.5f,  0.2f, -1.5f),
//    glm::vec3(-1.3f,  1.0f, -1.5f)
//    };
//    // 灯光立方体位置
//    glm::vec3 pointLightPositions[] = {
//    glm::vec3(0.7f,  0.2f,  2.0f),
//    glm::vec3(2.3f, -3.3f, -4.0f),
//    glm::vec3(-4.0f,  2.0f, -12.0f),
//    glm::vec3(0.0f,  0.0f, -3.0f)
//    };
//
//    // 首先配置立方体的VAO和VBO
//    unsigned int VBO;
//    unsigned int cubeVAO;
//    glGenVertexArrays(1, &cubeVAO);
//    glGenBuffers(1, &VBO);
//
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // 将顶点数据复制到缓冲区
//
//    glBindVertexArray(cubeVAO);
//
//    // 位置属性
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(0); // 启用顶点属性
//
//    // 各点法线属性
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
//    glEnableVertexAttribArray(1); // 启用顶点属性
//
//    // 纹理坐标属性
//    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
//    glEnableVertexAttribArray(2); // 启用顶点属性
//
//    // 其次配置灯立方体的VAO（VBO保持不变）
//    unsigned int lightCubeVAO;
//    glGenVertexArrays(1, &lightCubeVAO);
//    glBindVertexArray(lightCubeVAO);
//
//    // 由于灯立方体与之前的VBO绑定相同的顶点数据，我们只需要绑定VBO（不需要复制数据）
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(0);
//
//    // 加载漫反射贴图纹理
//    unsigned int diffuseMap = loadTexture("res/texture/container2.png");
//    // 加载镜面反射贴图纹理
//    unsigned int specularMap = loadTexture("res/texture/container2_specular.png");
//
//    // 激活着色器纹理单元
//    lightingShader.use();
//    lightingShader.setInt("material.diffuse", 0); // 告诉着色器每个采样器属于哪个纹理单元(只需要设置一次)
//    lightingShader.setInt("material.specular", 1);
//
//    // 主循环
//    while (!glfwWindowShouldClose(window))
//    {
//        // 计算帧数时间差，根据帧数大小调整移动速度
//        //float currentFrame = glfwGetTime();
//        float currentFrame = static_cast<float>(glfwGetTime());
//        deltaTime = currentFrame - lastFrame;
//        lastFrame = currentFrame;
//
//        // 输入
//        processInput(window);
//
//        // 渲染指令
//        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 设置清除颜色
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 清除颜色缓冲
//
//        // 移动光源
//        //lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
//        //lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;
//
//        // 激活着色器
//        lightingShader.use();
//        lightingShader.setVec3("viewPos", camera.Position);
//        lightingShader.setFloat("material.shininess", 32.0f);
//        // 定向光
//        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
//        lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
//        lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
//        // 点光源 1
//        lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
//        lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
//        lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
//        lightingShader.setFloat("pointLights[0].constant", 1.0f);
//        lightingShader.setFloat("pointLights[0].linear", 0.09f);
//        lightingShader.setFloat("pointLights[0].quadratic", 0.032f);
//        // 点光源 2
//        lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
//        lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
//        lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
//        lightingShader.setFloat("pointLights[1].constant", 1.0f);
//        lightingShader.setFloat("pointLights[1].linear", 0.09f);
//        lightingShader.setFloat("pointLights[1].quadratic", 0.032f);
//        // 点光源 3
//        lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
//        lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
//        lightingShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
//        lightingShader.setFloat("pointLights[2].constant", 1.0f);
//        lightingShader.setFloat("pointLights[2].linear", 0.09f);
//        lightingShader.setFloat("pointLights[2].quadratic", 0.032f);
//        // 点光源 4
//        lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
//        lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
//        lightingShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
//        lightingShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
//        lightingShader.setFloat("pointLights[3].constant", 1.0f);
//        lightingShader.setFloat("pointLights[3].linear", 0.09f);
//        lightingShader.setFloat("pointLights[3].quadratic", 0.032f);
//        // 聚光灯
//        lightingShader.setVec3("spotLight.position", camera.Position);
//        lightingShader.setVec3("spotLight.direction", camera.Front);
//        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
//        lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
//        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
//        lightingShader.setFloat("spotLight.constant", 1.0f);
//        lightingShader.setFloat("spotLight.linear", 0.09f);
//        lightingShader.setFloat("spotLight.quadratic", 0.032f);
//        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
//        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
//
//        // 创建投影/视图矩阵
//        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
//        glm::mat4 view = camera.GetViewMatrix();
//        lightingShader.setMat4("projection", projection);
//        lightingShader.setMat4("view", view);
//
//        // 世界变换矩阵
//        glm::mat4 model = glm::mat4(1.0f);
//        lightingShader.setMat4("model", model);
//
//        // 绑定漫反射贴图
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, diffuseMap);
//
//        // 绑定镜面反射贴图
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, specularMap);
//
//        // 混合光源绘制多个立方体
//        glBindVertexArray(cubeVAO);
//        for (unsigned int i = 0; i < 10; i++)
//        {
//            // calculate the model matrix for each object and pass it to shader before drawing
//            glm::mat4 model = glm::mat4(1.0f);
//            model = glm::translate(model, cubePositions[i]);
//            float angle = 20.0f * i;
//            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
//            lightingShader.setMat4("model", model);
//
//            glDrawArrays(GL_TRIANGLES, 0, 36);
//        }
//
//		// 设置等立方体着色器并传递变换矩阵
//        lightCubeShader.use();
//        lightCubeShader.setMat4("projection", projection);
//        lightCubeShader.setMat4("view", view);
//		// 也绘制灯立方体
//        glBindVertexArray(lightCubeVAO);
//        for (unsigned int i = 0; i < 4; i++)
//        {
//            model = glm::mat4(1.0f);
//            model = glm::translate(model, pointLightPositions[i]);
//            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
//            lightCubeShader.setMat4("model", model);
//            glDrawArrays(GL_TRIANGLES, 0, 36);
//        }
//
//        // 检查并调用事件，交换缓冲
//        glfwSwapBuffers(window);
//        glfwPollEvents();          // 处理事件
//    }
//    // 删除VAO、VBO和着色器程序
//    glDeleteVertexArrays(1, &cubeVAO);
//    glDeleteVertexArrays(1, &lightCubeVAO);
//    glDeleteBuffers(1, &VBO);
//
//
//    glfwTerminate(); // 终止GLFW库
//    return 0;
//}
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
