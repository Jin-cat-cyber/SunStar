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
//#include "camera.h"
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
//
//const unsigned int SCR_WIDTH = 960; // 窗口宽度
//const unsigned int SCR_HEIGHT = 600; // 窗口高度
//
//// 摄像机相关
//Camera camera(glm::vec3(0.0f, 3.0f, 90.0f));
//float lastX = SCR_WIDTH / 2.0F;
//float lastY = SCR_HEIGHT / 2.0F;
//bool firstMouse = true;
//
//
//// 设置帧数渲染时间
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
//
//    // 创建着色器对象
//    Shader planetshader("res/shader/InstancingShader/instancingVER.shader", "res/shader/InstancingShader/instancingFRAG.shader");
//	Shader asteroidShader("res/shader/InstancingShader/aster_ver.shader", "res/shader/InstancingShader/aster_frag.shader");
//    Shader starShader("res/shader/StarShader/star_ver.shader", "res/shader/StarShader/star_frag.shader");
//	Shader spaceboxShader("res/shader/SkyBoxShader/SkyBox_ver.shader", "res/shader/SkyBoxShader/SkyBox_frag.shader");
//
//	Model rock("res/model/rock/rock.obj");
//	Model planet("res/model/planet/planet.obj");
//
//
//
//    // 生成一个大型的半随机模型变换矩阵列表
//    // ------------------------------------------------------------------
//    unsigned int amount = 100000;
//	glm::mat4* modelMatrices;
//	modelMatrices = new glm::mat4[amount];
//	srand(static_cast<unsigned int>(glfwGetTime())); // 生成随机种子
//	float radius = 150.0;
//    float offset = 25.0f;
//    for (unsigned int i = 0; i < amount; i++)
//    {   
//		glm::mat4 model = glm::mat4(1.0f);
//		// 1.  平移  ：沿圆周位移，'半径'在范围内 [-offset, offset]
//		float angle = (float)i / (float)amount * 360.0f;
//        float displacement = (rand() % (int)(2 * offset * 100)) / 100.00f - offset;
//		float x = sin(angle) * radius + displacement;
//		displacement = (rand() % (int)(2 * offset * 100)) / 100.00f - offset;
//		float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
//		displacement = (rand() % (int)(2 * offset * 100)) / 100.00f - offset;
//		float z = cos(angle) * radius + displacement;
//        
//		model = glm::translate(model, glm::vec3(x, y, z));
//
//		// 2.  缩放  ：在 0.05 和 0.25f 之间进行缩放 
//		float scale = static_cast<float>((rand() % 20) / 100.0 + 0.05); // 在 0.05 和 0.25f 之间缩放
//		model = glm::scale(model, glm::vec3(scale));
//
//		// 3.  旋转  ：围绕一个（半）随机选取的旋转轴向量进行随机旋转。
//		float rotAngle = static_cast<float>((rand() % 360));
//		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
//
//		// 4. 现在添加到矩阵列表中
//		modelMatrices[i] = model;
//    }
//
//	// 设置实例化顶点属性
//	unsigned int buffer;
//	glGenBuffers(1, &buffer);
//	glBindBuffer(GL_ARRAY_BUFFER, buffer);
//	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
//
//    // set transformation matrices as an instance vertex attribute (with divisor 1)
//    // note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
//    // normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
//    // 
//    for (unsigned int i = 0; i < rock.meshes.size(); i++)
//    {
//		unsigned int VAO = rock.meshes[i].VAO;
//		glBindVertexArray(VAO);
//		// 设置顶点属性指针
//		glEnableVertexAttribArray(3);
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
//	// 程序化生成恒星顶点数据
//    // 球体顶点数据
//    std::vector<float> starVertices;
//    std::vector<unsigned int> starIndices;
//	// 生成球体顶点数据（可以使用UV球体或其他方法）
//    const unsigned int X_SEGMENTS = 64;
//	const unsigned int Y_SEGMENTS = 64;
//	const float PI = 3.14159265359f;
//
//	// 生成球体顶点数据
//    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
//    {
//        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
//        {
//            float xSegment = (float)x / (float)X_SEGMENTS;
//            float ySegment = (float)y / (float)Y_SEGMENTS;
//			float xPos = cos(xSegment * 2.0f * PI) * sin(ySegment * PI);
//            float yPos = cos(ySegment * PI);
//            float zPos = sin(xSegment * 2.0f * PI) * sin(ySegment * PI);
//            // 位置
//            starVertices.push_back(xPos);
//            starVertices.push_back(yPos);
//			starVertices.push_back(zPos);
//            // 法线
//            starVertices.push_back(xPos);
//            starVertices.push_back(yPos);
//            starVertices.push_back(zPos);
//			// 纹理坐标
//            starVertices.push_back(xSegment);
//			starVertices.push_back(ySegment);
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
//			starIndices.push_back(y * (X_SEGMENTS + 1) + x + 1);
//        }
//    }
//    
//	// 恒星 VAO, VBO, EBO
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
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
//    // 纹理坐标属性
//	glEnableVertexAttribArray(2);
//	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
//
//
//	// 天空盒顶点数据
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
//	// spacebox VAO, VBO
//    unsigned int skyboxVAO, skyboxVBO;
//    glGenVertexArrays(1, &skyboxVAO);
//    glGenBuffers(1, &skyboxVBO);
//    glBindVertexArray(skyboxVAO);
//    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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
//    //std::vector<std::string> face
//    //{
//    //    ("res/texture/skybox/right.jpg"),
//    //    ("res/texture/skybox/left.jpg"),
//    //    ("res/texture/skybox/top.jpg"),
//    //    ("res/texture/skybox/bottom.jpg"),
//    //    ("res/texture/skybox/front.jpg"),
//    //    ("res/texture/skybox/back.jpg")
//    //};
//
//    unsigned int cubemapTexture = loadCubemap(face);
//
//	// 设置光源位置
//    glm::vec3 pointSunPositions = glm::vec3(-50.0f, 0.0f, -200.0f);
//	glm::vec3 planetPosition = glm::vec3(0.0f, -3.0f, 0.0f);
//
//    // 加载漫反射贴图纹理
//    unsigned int diffuseMap = loadTexture("res/model/backpack/diffuse.jpg");
//    // 加载镜面反射贴图纹理
//    // unsigned int specularMap = loadTexture("res/model/backpack/specular.jpg");
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
//		// 计算帧时间
//        float currentFrame = static_cast<float>(glfwGetTime());
//        deltaTime = currentFrame - lastFrame;
//        lastFrame = currentFrame;
//        // 输入管理
//        processInput(window);
//        // 渲染
//        // ------
//        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//        // 激活着色器
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
//        planetshader.setVec3("pointLights[0].diffuse", 80.0f, 80.0f, 60.0f);   // 增大
//        planetshader.setVec3("pointLights[0].specular", 120.0f, 120.0f, 100.0f);  // 增大
//        planetshader.setFloat("pointLights[0].constant", 1.0f);
//        planetshader.setFloat("pointLights[0].linear", 0.005f);        // 原来是0.09，减小
//        planetshader.setFloat("pointLights[0].quadratic", 0.0001f);    // 原来是0.032，减小
//
//        // 配置变换矩阵
//        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
//        //glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
//        glm::mat4 view = camera.GetViewMatrix();
//		planetshader.setMat4("projection", projection);
//		planetshader.setMat4("view", view);
//
//		// 绘制行星
//        glm::mat4 model = glm::mat4(1.0f);
//		model = glm::translate(model, planetPosition);
//		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
//		planetshader.setMat4("model", model);
//		// 计算法线矩阵并传递给着色器
//		//glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));
//		//planetshader.setMat3("normalMatrix", normalMat);
//		planet.Draw(planetshader);
//
//
//		// 设置小行星着色器并绘制
//        asteroidShader.use();
//        asteroidShader.setMat4("projection", projection);
//        asteroidShader.setMat4("view", view);
//
//        asteroidShader.use();
//        asteroidShader.setInt("material.diffuse", 0);
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, rock.textures_loaded[0].id);
//        //asteroidShader.setInt("material.specular", 1);
//		// 设置小行星着色器并绘制
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
//        asteroidShader.setVec3("pointLights[0].diffuse", 80.0f, 80.0f, 60.0f);   // 增大
//        asteroidShader.setVec3("pointLights[0].specular", 120.0f, 120.0f, 100.0f);  // 增大
//        asteroidShader.setFloat("pointLights[0].constant", 1.0f);
//        asteroidShader.setFloat("pointLights[0].linear", 0.005f);        // 原来是0.09，减小
//        asteroidShader.setFloat("pointLights[0].quadratic", 0.0001f);    // 原来是0.032，减小
//
//		// 绘制小行星
//        for(unsigned int i = 0; i < rock.meshes.size(); i++)
//        {
//
//			glBindVertexArray(rock.meshes[i].VAO);
//			glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(rock.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, amount);
//			glBindVertexArray(0);
//		}
//
//        starShader.use();
//        glm::mat4 starModel = glm::mat4(1.0f);
//		starModel = glm::translate(starModel, pointSunPositions); // 恒星位置
//		starModel = glm::scale(starModel, glm::vec3(15.0f));  // 恒星大小
//        starShader.setMat4("model", starModel);
//		starShader.setMat4("view", view);
//		starShader.setMat4("projection", projection);
//        glBindVertexArray(starVAO);
//        for(unsigned int i = 0; i < 1000; i++)
//        {
//            glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(starIndices.size()), GL_UNSIGNED_INT, 0);
//		}
//        glBindVertexArray(0);
//
//        // 最后绘制天空盒
//        glDepthFunc(GL_LEQUAL);  // 更改深度函数，允许绘制
//        spaceboxShader.use();
//        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // 去除平移部分
//        spaceboxShader.setMat4("view", view);
//        spaceboxShader.setMat4("projection", projection);
//
//        glBindVertexArray(skyboxVAO);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
//        glDrawArrays(GL_TRIANGLES, 0, 36);
//        glBindVertexArray(0);
//        glDepthFunc(GL_LESS); // 恢复默认深度函数
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//	
//	glDeleteVertexArrays(1, &starVAO);
//    glDeleteBuffers(1, &starVBO);
//    glDeleteVertexArrays(1, &skyboxVAO);
//    glDeleteBuffers(1, &skyboxVBO);
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
//
//
//
