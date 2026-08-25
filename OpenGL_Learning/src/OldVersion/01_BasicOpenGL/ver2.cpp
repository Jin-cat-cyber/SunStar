//#include <glad/glad.h>
//#include <glfw3.h>
//
//#include "Shader.h" // 包含自定义着色器类
//
//#include <iostream>
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
//        // positions         // colors
//         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
//        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
//         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top 
//
//    };
//
//    //// 索引数据
//    //unsigned int indices[] = { // 索引数据
//    //    0, 1, 3, // 第一个三角形
//    //    1, 2, 3  // 第二个三角形
//    //};
//
//    unsigned int VBO, VAO;
//    // unsigned int EBO;
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//    //glGenBuffers(1, &EBO);
//
//    glBindVertexArray(VAO); // 绑定VAO
//
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // 将顶点数据复制到缓冲区
//
//    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // 将索引数据复制到缓冲区
//
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // 设置顶点属性指针
//    glEnableVertexAttribArray(0); // 启用顶点属性
//
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // 设置颜色属性指针
//    glEnableVertexAttribArray(1); // 启用颜色属性
//    //glBindBuffer(GL_ARRAY_BUFFER, 0); // 解绑VBO
//    //glBindVertexArray(0); // 解绑VAO
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
//		ourShader.use(); // 使用着色器程序
//        glBindVertexArray(VAO);
//        glDrawArrays(GL_TRIANGLES, 0, 3);
//
//        // 检查并调用事件，交换缓冲
//        glfwSwapBuffers(window);
//        glfwPollEvents();          // 处理事件
//    }
//    // 删除VAO、VBO和着色器程序
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//    //glDeleteBuffers(1, &EBO);
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