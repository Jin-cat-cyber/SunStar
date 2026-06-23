//#include <glad/glad.h>
//#include <glfw3.h>
//
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
////using namespace std;
//
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height); // 窗口大小回调函数
//void processInput(GLFWwindow* window); // 输入检查函数
//
//const unsigned int SCR_WIDTH = 800; // 窗口宽度
//const unsigned int SCR_HEIGHT = 600; // 窗口高度
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
//
//    // 初始化GLAD，管理OpenGL函数指针
//    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//    {
//        std::cout << "Failed to initialize GLAD" << std::endl;
//        return -1;
//    }
//
//    Shader ourShader("res/shader/vertexShader.shader", "res/shader/fragmentShader.shader");
//
//    // 顶点数据
//    float vertices[] = {
//        // positions          // colors           // texture coords
//        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
//        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
//       -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
//       -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
//
//    };
//
//    // 索引数据
//    unsigned int indices[] = { // 索引数据
//        0, 1, 3, // 第一个三角形
//        1, 2, 3  // 第二个三角形
//    };
//
//    unsigned int VBO;
//    unsigned int VAO;
//    unsigned int EBO;
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//    glGenBuffers(1, &EBO);
//
//    glBindVertexArray(VAO); // 绑定VAO
//
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // 将顶点数据复制到缓冲区
//
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // 将索引数据复制到缓冲区
//
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // 设置顶点属性指针
//    glEnableVertexAttribArray(0); // 启用顶点属性
//
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // 设置颜色属性指针
//    glEnableVertexAttribArray(1); // 启用颜色属性
//
//	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // 设置纹理坐标属性指针
//	glEnableVertexAttribArray(2); // 启用纹理坐标属性
//
//    // 加载纹理
//    unsigned int texture1;
//	glGenTextures(1, &texture1);
//	glBindTexture(GL_TEXTURE_2D, texture1); // 绑定纹理对象
//    
//	// 设置纹理参数
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // 设置纹理环绕方式
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // 设置纹理环绕方式
//	// 设置纹理过滤方式
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // 线性多级渐远纹理 过滤
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);               // 线性过滤
//	// 加载纹理图像
//    int width, height, nrChannels;
//    stbi_set_flip_vertically_on_load(true);// 告诉stbi库，加载图片y轴纹理坐标的y轴
//	unsigned char* data = stbi_load("res/texture/container.jpg", &width, &height, &nrChannels, 0);
//
//    if(data)
//    {
//        // 创建纹理
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//        glGenerateMipmap(GL_TEXTURE_2D); // 生成多级渐远纹理
//    }
//    else
//    {
//        std::cout << "Failed to load texture" << std::endl;
//	}
//	stbi_image_free(data); // 释放图像数据
//
//
//    unsigned int texture2;
//    glGenTextures(1, &texture2);
//    glBindTexture(GL_TEXTURE_2D, texture2);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // 设置纹理环绕方式
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // 设置纹理环绕方式
//    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // 线性多级渐远纹理 过滤
//    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);               // 线性过滤
//
//    data = stbi_load("res/texture/awesomeface.png", &width, &height, &nrChannels, 0);
//    if (data)
//    {
//        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
//        glGenerateMipmap(GL_TEXTURE_2D);
//    }
//    else
//    {
//        std::cout << "Failed to load texture" << std::endl;
//    }
//    stbi_image_free(data);
//
//
//    ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
//    // either set it manually like so:
//    glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
//    // or set it via the texture class
//    ourShader.setInt("texture2", 1);
//
//
//    // 主循环
//    while (!glfwWindowShouldClose(window))
//    {
//        // 输入
//        processInput(window);
//
//        // 渲染指令
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // 设置清除颜色
//        glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲
//
//        // 绑定纹理到纹理单元
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, texture1);
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, texture2);
//
//        glm::mat4 transform = glm::mat4(1.0f);
//        transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));  // 平移到右下角
//        transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f)); // 随时间不断旋转
//
//        // 创建变换矩阵
//        ourShader.use(); // 使用着色器程序
//        unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
//        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
//
//        glBindVertexArray(VAO);
//        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//
//        // 检查并调用事件，交换缓冲
//        glfwSwapBuffers(window);
//        glfwPollEvents();          // 处理事件
//    }
//    // 删除VAO、VBO和着色器程序
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//    glDeleteBuffers(1, &EBO);
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
//}
//
//// 输入检查函数
//void processInput(GLFWwindow* window)
//{
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // 检查是否按下ESC键
//        glfwSetWindowShouldClose(window, true); // 设置窗口关闭标志
//}