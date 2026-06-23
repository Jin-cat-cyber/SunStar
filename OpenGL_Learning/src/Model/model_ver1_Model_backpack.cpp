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
//	// 纹理y轴翻转(因为OpenGL的y轴坐标是从下往上，而图片的y轴坐标是从上往下)
//	stbi_set_flip_vertically_on_load(true); 
//
//    //加载深度缓冲
//    glEnable(GL_DEPTH_TEST);
//
//    // 创建着色器对象
//    Shader ourShader("res/shader/ModelShader/model_ver.shader", "res/shader/ModelShader/model_frag.shader");
//
//	// 加载模型
//    Model ourModel("res/model/backpack/backpack.obj");
//
//
//    
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
//		ourShader.use();
//
//		glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); 
//		glm::mat4 view = camera.GetViewMatrix();
//		ourShader.setMat4("projection", projection);
//		ourShader.setMat4("view", view);
//
//        // 渲染模型
//		glm::mat4 model = glm::mat4(1.0f);
//		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // 向下移动到屏幕底部中央位置
//		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// 缩放
//		ourShader.setMat4("model", model);
//		ourModel.Draw(ourShader);
//
//        // 检查并调用事件，交换缓冲
//        glfwSwapBuffers(window);
//        glfwPollEvents();          // 处理事件
//    }
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
