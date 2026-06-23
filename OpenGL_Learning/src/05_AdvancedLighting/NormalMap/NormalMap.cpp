//#include <glad/glad.h>
//#include <glfw3.h>
//#include <assimp/config.h>
//#include <assimp/revision.h>
//
//#include "Shader.h" // 包含自定义着色器类
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>
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
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height); // 窗口大小回调函数
//void processInput(GLFWwindow* window); // 输入检查函数
//void mouse_callback(GLFWwindow* window, double xpos, double ypos); // 鼠标 移动 回调函数
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // 鼠标 滚轮 回调函数
//unsigned int loadTexture(char const* path); // 纹理加载函数
//void renderQuad();
//
//const unsigned int SCR_WIDTH = 960; // 窗口宽度
//const unsigned int SCR_HEIGHT = 600; // 窗口高度
////bool shadows = true; // 是否开启阴影
////bool shadowsKeyPressed = false; // 是否按下了空格键
//// bool blinn = false; // 是否开启Blinn-Phong光照模型
//// bool blinnKeyPressed = false;   // 是否按下了B键
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
//    // 纹理y轴翻转(因为OpenGL的y轴坐标是从下往上，而图片的y轴坐标是从上往下)
//    stbi_set_flip_vertically_on_load(true);
//
//    //加载深度缓冲
//    glEnable(GL_DEPTH_TEST);
//  /*  glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/
//
//    // 创建着色器对象    
//	Shader shader("res/shader/Advanced_lighting/NormalMap/Normal_ver.shader", "res/shader/Advanced_lighting/NormalMap/Normal_frag.shader");
//
//    // 加载贴图
//    unsigned int diffuseMap = loadTexture("res/texture/NormalMap/brickwall.jpg");
//    unsigned int normalMap = loadTexture("res/texture/NormalMap/brickwall_normal.jpg");
//
//    // 着色器配置
//    // --------------------
//    shader.use();
//    shader.setInt("diffuseMap", 0);
//	shader.setInt("normalMap", 1);
//
//    // 光源位置
//    // -------------
//    glm::vec3 lightPos(0.5f, 1.0f, 0.3f);
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
//        // 渲染
//        // ------
//        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//        // 配置 裁剪矩阵和视图矩阵
//        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
//        glm::mat4 view = camera.GetViewMatrix();
//        shader.use();
//        shader.setMat4("projection", projection);
//        shader.setMat4("view", view);
//        // 渲染法线贴图四边形
//        glm::mat4 model = glm::mat4(1.0f);
//        model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0))); // rotate the quad to show normal mapping from multiple directions
//        shader.setMat4("model", model);
//        shader.setVec3("viewPos", camera.Position);
//        shader.setVec3("lightPos", lightPos);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, diffuseMap);
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, normalMap);
//        renderQuad();
//
//        // 光照资源
//        model = glm::mat4(1.0f);
//		model = glm::translate(model, lightPos);
//		model = glm::scale(model, glm::vec3(0.1f));
//		shader.setMat4("model", model);
//        renderQuad();
//        //shader.use();
//        //glm::mat4 model = glm::mat4(1.0f);
//        //glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
//        //glm::mat4 view = camera.GetViewMatrix();
//        //shader.setMat4("projection", projection);
//        //shader.setMat4("view", view);
//        //shader.setMat4("model", model);
//        //// 设置灯光uniform
//        //shader.setVec3("viewPos", camera.Position);
//        //shader.setVec3("lightPos", lightPos);
//        //shader.setInt("blinn", blinn);
//
//        //// 定向光
//        //shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
//        //shader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
//        //shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
//        //shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
//        //// 点光源 1
//        //shader.setVec3("pointLights[0].position", lightPos);
//        //shader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
//        //shader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
//        //shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
//        //shader.setFloat("pointLights[0].constant", 1.0f);
//        //shader.setFloat("pointLights[0].linear", 0.09f);
//        //shader.setFloat("pointLights[0].quadratic", 0.032f);
//        //shader.setVec3("spotLight.position", camera.Position);
//        //shader.setVec3("spotLight.direction", camera.Front);
//        //shader.setVec3("spotLight.ambient", 0.5f, 0.5f, 0.5f);
//        //shader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
//        //shader.setVec3("spotLight.specular", 0.3f, 0.3f, 0.3f);
//        //shader.setFloat("spotLight.constant", 1.0f);
//        //shader.setFloat("spotLight.linear", 0.09f);
//        //shader.setFloat("spotLight.quadratic", 0.032f);
//        //shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
//        //shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
//
//
//        ////std::cout << (blinn ? "Blinn-Phong" : "Phong") << std::endl;
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    glfwTerminate(); // 清理并关闭GLFW
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
//
//   /* if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinnKeyPressed)
//    {
//        blinn = !blinn;
//        blinnKeyPressed = true;
//    }
//    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
//    {
//        blinnKeyPressed = false;
//    }*/
//
//    //if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !shadowsKeyPressed)
//    //{
//    //    shadows = !shadows;
//    //    shadowsKeyPressed = true;
//    //}
//    //if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
//    //{
//    //    shadowsKeyPressed = false;
//    //}
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
////// 渲染
//unsigned int quadVAO = 0;
//unsigned int quadVBO;
//// 错误的
//void renderQuad()
//{
//    if (quadVAO == 0)
//    {
//        // positions
//        glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
//        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
//        glm::vec3 pos3(1.0f, -1.0f, 0.0f);
//        glm::vec3 pos4(1.0f, 1.0f, 0.0f);
//        // texture coordinates
//        glm::vec2 uv1(0.0f, 1.0f);
//        glm::vec2 uv2(0.0f, 0.0f);
//        glm::vec2 uv3(1.0f, 0.0f);
//        glm::vec2 uv4(1.0f, 1.0f);
//        // normal vector
//        glm::vec3 nm(0.0f, 0.0f, 1.0f);
//
//        // 计算每个三角形的 T/B 向量
//        glm::vec3 tangent1, bitangent1;
//        glm::vec3 tangent2, bitangent2;
//
//        // 三角形1
//        //————————
//        glm::vec3 edge1 = pos2 - pos1;
//        glm::vec3 edge2 = pos3 - pos1;
//        glm::vec2 deltaUV1 = uv2 - uv1;
//        glm::vec2 deltaUV2 = uv3 - uv1;
//
//        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
//        // T1
//        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
//        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
//        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
//        // B1
//        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
//        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
//        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
//
//        // 三角形2
//        //————————
//        edge1 = pos3 - pos1;
//        edge2 = pos4 - pos1;
//        deltaUV1 = uv3 - uv1;
//        deltaUV2 = uv4 - uv1;
//
//        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
//        // T2
//        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
//        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
//        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
//        // B2
//        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
//        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
//        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
//
//
//        float quadVertices[] = {
//            // 位置                  // 法线           // 纹理UV      // T                                // B
//            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
//            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
//            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
//
//            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
//            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
//            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
//        };
//
//        // 配置四边形VAO
//        glGenVertexArrays(1, &quadVAO);
//        glGenBuffers(1, &quadVBO);
//        glBindVertexArray(quadVAO);
//        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
//        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
//        glEnableVertexAttribArray(0);
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
//        glEnableVertexAttribArray(1);
//        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
//        glEnableVertexAttribArray(2);
//        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
//        glEnableVertexAttribArray(3);
//        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
//        glEnableVertexAttribArray(4);
//        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
//    }
//    glBindVertexArray(quadVAO);
//    glDrawArrays(GL_TRIANGLES, 0, 6);
//    glBindVertexArray(0);
//}
//
////// 正确的
////void renderQuad()
////{
////    if (quadVAO == 0)
////    {
////        // positions
////        glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
////        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
////        glm::vec3 pos3(1.0f, -1.0f, 0.0f);
////        glm::vec3 pos4(1.0f, 1.0f, 0.0f);
////        // texture coordinates
////        glm::vec2 uv1(0.0f, 1.0f);
////        glm::vec2 uv2(0.0f, 0.0f);
////        glm::vec2 uv3(1.0f, 0.0f);
////        glm::vec2 uv4(1.0f, 1.0f);
////        // normal vector
////        glm::vec3 nm(0.0f, 0.0f, 1.0f);
////
////        // calculate tangent/bitangent vectors of both triangles
////        glm::vec3 tangent1, bitangent1;
////        glm::vec3 tangent2, bitangent2;
////        // triangle 1
////        // ----------
////        glm::vec3 edge1 = pos2 - pos1;
////        glm::vec3 edge2 = pos3 - pos1;
////        glm::vec2 deltaUV1 = uv2 - uv1;
////        glm::vec2 deltaUV2 = uv3 - uv1;
////
////        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
////
////        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
////        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
////        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
////
////        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
////        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
////        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
////
////        // triangle 2
////        // ----------
////        edge1 = pos3 - pos1;
////        edge2 = pos4 - pos1;
////        deltaUV1 = uv3 - uv1;
////        deltaUV2 = uv4 - uv1;
////
////        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
////
////        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
////        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
////        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
////
////
////        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
////        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
////        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
////
////
////        float quadVertices[] = {
////            // positions            // normal         // texcoords  // tangent                          // bitangent
////            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
////            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
////            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
////
////            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
////            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
////            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
////        };
////        // configure plane VAO
////        glGenVertexArrays(1, &quadVAO);
////        glGenBuffers(1, &quadVBO);
////        glBindVertexArray(quadVAO);
////        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
////        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
////        glEnableVertexAttribArray(0);
////        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
////        glEnableVertexAttribArray(1);
////        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
////        glEnableVertexAttribArray(2);
////        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
////        glEnableVertexAttribArray(3);
////        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
////        glEnableVertexAttribArray(4);
////        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
////    }
////    glBindVertexArray(quadVAO);
////    glDrawArrays(GL_TRIANGLES, 0, 6);
////    glBindVertexArray(0);
////}