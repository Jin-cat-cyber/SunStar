//#include <glad/glad.h>
//#include <glfw3.h>
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
//// 顶点着色器
//const char* vertexShaderSource = R"(
//        #version 330 core
//        layout(location = 0) in vec3 aPos; // 顶点位置属性
//        layout(location = 1) in vec3 aColor; // 顶点颜色属性
//
//        out vec3 ourColor; // 输出颜色
//
//        void main()
//        {
//            gl_Position = vec4(aPos, 1.0); // 设置顶点位置
//            ourColor = aColor; // 传递顶点颜色
//        }
//    )";
//
//// 片段着色器
//const char* fragmentShaderSource = R"(
//        #version 330 core
//        out vec4 FragColor; // 输出颜色
//        in vec3 ourColor; // 接收来自顶点着色器的颜色
//
//        void main()
//        {
//            FragColor = vec4(ourColor, 1.0); // 设置片段颜色
//        }
//    )";
//
//
//int main()
//{
//	glfwInit(); // 初始化GLFW库
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // 设置OpenGL版本：主版本号
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // 设置OpenGL版本：次版本号
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 使用核心模式
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
//	glfwMakeContextCurrent(window); // 设置当前窗口为上下文
//	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // 设置窗口大小回调函数
//
//	// 初始化GLAD，管理OpenGL函数指针
//    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//    {
//        std::cout << "Failed to initialize GLAD" << std::endl;
//        return -1;
//    }
//
//    // #着色器部分
//    unsigned int vertexShader;
//	vertexShader = glCreateShader(GL_VERTEX_SHADER); // 创建顶点着色器
//	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // 设置着色器源代码
//    /* glShaderSource函数把要编译的着色器对象作为第一个参数。
//        第二参数指定了传递的源码字符串数量，这里只有一个。
//        第三个参数是顶点着色器真正的源码，第四个参数我们先设置为NULL。*/
//    glCompileShader(vertexShader); // 编译顶点着色器
//
//    int success;
//    char infolog[512];
//	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); // 检查编译状态
//
//    if(!success) // 如果编译失败
//    {
//        glGetShaderInfoLog(vertexShader, 512, NULL, infolog); // 获取错误信息
//        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infolog << std::endl;
//	}
//    else 
//    {
//        std::cout << "Vertex Shader compiled successfully!" << std::endl;
//	}   
//
//    unsigned int fragmentShader;
//	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // 创建片段着色器
//	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // 设置片段着色器源代码
//	glCompileShader(fragmentShader); // 编译片段着色器
//    // 检查片段着色器是否编译成功
//    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); // 检查编译状态
//    if (!success) // 如果编译失败
//    {
//        glGetShaderInfoLog(fragmentShader, 512, NULL, infolog); // 获取错误信息
//        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infolog << std::endl;
//    }
//    else
//    {
//        std::cout << "Fragment Shader compiled successfully!" << std::endl;
//    }
//
//    // 创建程序对象，绑定着色器，将两个着色器链接
//    unsigned int shaderProgram;
//	shaderProgram = glCreateProgram(); // 创建程序对象
//    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, fragmentShader);
//    glLinkProgram(shaderProgram); 
//	// 检查程序对象是否链接成功
//	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success); // 检查链接状态
//    if (!success)
//    {
//        glGetProgramInfoLog(shaderProgram, 512, NULL, infolog); // 获取错误信息}
//		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infolog << std::endl;
//    }
//    else
//    {
//		std::cout << "Shader Program linked successfully!" << std::endl;
//    }
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
//
//	// 顶点数据
//    float vertices[] = {
//        // positions         // colors
//         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
//        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
//         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top 
//
//    };
//
//
//	// 索引数据
//    unsigned int indices[] = { // 索引数据
//        0, 1, 3, // 第一个三角形
//        1, 2, 3  // 第二个三角形
//	};
//
//    unsigned int VBO, VAO;
//   // unsigned int EBO;
//    glGenVertexArrays(1, &VAO); 
//    glGenBuffers(1, &VBO);
//	//glGenBuffers(1, &EBO);
//
//	glBindVertexArray(VAO); // 绑定VAO
//
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // 将顶点数据复制到缓冲区
//
//    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // 将索引数据复制到缓冲区
//
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // 设置顶点属性指针
//	glEnableVertexAttribArray(0); // 启用顶点属性
//
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // 设置颜色属性指针
//	glEnableVertexAttribArray(1); // 启用颜色属性
//	//glBindBuffer(GL_ARRAY_BUFFER, 0); // 解绑VBO
//	//glBindVertexArray(0); // 解绑VAO
//
//	glUseProgram(shaderProgram); // 使用着色器程序
//
//    // 主循环
//    while (!glfwWindowShouldClose(window)) 
//    {
//        // 输入
//		processInput(window);
//
//		// 渲染指令
//		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // 设置清除颜色
//		glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲
//
//        
//        glBindVertexArray(VAO);
//		glDrawArrays(GL_TRIANGLES, 0, 3);
//		
//        // 检查并调用事件，交换缓冲
//		glfwSwapBuffers(window); 
//		glfwPollEvents();          // 处理事件
//    }
//    // 删除VAO、VBO和着色器程序
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//    //glDeleteBuffers(1, &EBO);
//    glDeleteProgram(shaderProgram);
//	
//	glfwTerminate(); // 终止GLFW库
//
//    return 0;
//}
//
//// 窗口回调函数
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//	glViewport(0, 0, width, height); // glViewport 用于设置视口大小
//}
//
//// 输入检查函数
//void processInput(GLFWwindow* window)
//{
//	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // 检查是否按下ESC键
//		glfwSetWindowShouldClose(window, true); // 设置窗口关闭标志
//}