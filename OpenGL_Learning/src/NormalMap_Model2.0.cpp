#include <glad/glad.h>
#include <glfw3.h>
#include <assimp/config.h>
#include <assimp/revision.h>

#include "Shader.h" // 包含自定义着色器类
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "Model.h"
//using namespace std;


void framebuffer_size_callback(GLFWwindow* window, int width, int height); // 窗口大小回调函数
void processInput(GLFWwindow* window); // 输入检查函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // 鼠标 移动 回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // 鼠标 滚轮 回调函数
unsigned int loadTexture(char const* path); // 纹理加载函数

const unsigned int SCR_WIDTH = 960; // 窗口宽度
const unsigned int SCR_HEIGHT = 600; // 窗口高度

// 摄像机相关
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0F;
float lastY = SCR_HEIGHT / 2.0F;
bool firstMouse = true;
bool cursorLocked = true;
bool tabKeyPressed = false;   // 用于检测 TAB 键的上升沿
bool gamma = true; // 是否开启伽马校正
bool gammaKeyPressed = false;   // 是否按下了G键

// 设置帧数渲染位置
float deltaTime = 0.0f;	// 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间

// 设置光源位置
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);


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

    // 创建着色器对象
    Shader shader("res/shader/Advanced_lighting/NormalMap/Normal_ver2.0.shader", "res/shader/Advanced_lighting/NormalMap/Normal_frag2.0.shader");
    Shader lightCubeShader("res/shader/LightShader/ver_light_cube.shader", "res/shader/LightShader/frag_light_cube.shader");

    // 加载模型
    Model ourModel("res/model/backpack/backpack.obj");

    // 立方体顶点数据
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f,         -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f,         -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f,         -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f,         -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f,         -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f,         -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,         1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,         1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,         1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,         1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,         1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,         1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,          0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,          0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,          0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,          0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,          0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,          0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,          0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,          0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,          0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,          0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,          0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,          0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,          0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,          0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,          0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,          0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,          0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,          0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,          0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,          0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,          0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,          0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,          0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,          0.0f,  0.0f, 1.0f
    };

    // 灯光立方体位置
    glm::vec3 pointLightPositions = glm::vec3(0.7f, 0.2f, 2.0f);

    // 灯立方体缓冲区对象
    unsigned int VBO;
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // 将顶点数据复制到缓冲区

    glBindVertexArray(lightCubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 加载漫反射贴图纹理
    unsigned int diffuseMap = loadTexture("res/model/backpack/diffuse.jpg");
    // 加载镜面反射贴图纹理
    unsigned int specularMap = loadTexture("res/model/backpack/specular.jpg");
    // 加载法线贴图纹理
    unsigned int normalMap = loadTexture("res/model/backpack/normal.png");

    // 激活着色器纹理单元
    shader.use();
    shader.setInt("diffuseMap", 0); // 告诉着色器每个采样器属于哪个纹理单元(只需要设置一次)
    shader.setInt("specularMap", 1);
    shader.setInt("normalMap", 2);
	


    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        // 计算帧数时间差，根据帧数大小调整移动速度
        //float currentFrame = glfwGetTime();
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 输入
        processInput(window);

        // 渲染指令
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 设置清除颜色
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 清除颜色缓冲



        // 激活着色器
        shader.use();
        shader.setVec3("viewPos", camera.Position);
        shader.setBool("gamma", gamma);
        //shader.setFloat("material.shininess", 32.0f);
        
        // 定向光
        /* shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		// 光色过冷
        shader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        */
		// 调整为暖色调
        shader.setVec3("dirLight.ambient", 0.18f, 0.16f, 0.12f);
        shader.setVec3("dirLight.diffuse", 0.55f, 0.48f, 0.35f);
        shader.setVec3("dirLight.specular", 0.3f, 0.3f, 0.3f);

        // 点光源 1
        shader.setVec3("pointLights[0].position", pointLightPositions);
		/* // 光色过冷
		shader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        shader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        */
        // 调整为暖色调
        shader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.04f);   // 很暗的暖色
        shader.setVec3("pointLights[0].diffuse", 0.6f, 0.5f, 0.4f);     // 偏暖，亮度适中
        shader.setVec3("pointLights[0].specular", 0.4f, 0.4f, 0.4f);    // 高光再低一点

        shader.setFloat("pointLights[0].constant", 1.0f);
        shader.setFloat("pointLights[0].linear", 0.09f);
        shader.setFloat("pointLights[0].quadratic", 0.032f);

        // 聚光灯
        shader.setVec3("spotLight.position", camera.Position);
        //shader.setVec3("spotLight.direction", camera.Front);
		// 光色过冷
        /*
        shader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        shader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        */
		// 调整为暖色调
        shader.setVec3("spotLight.ambient", 0.12f, 0.10f, 0.07f);      // 少许填充光，避免中心以外一片黑
        shader.setVec3("spotLight.diffuse", 0.6f, 0.55f, 0.4f);        // 暖黄光，强度大大降低
        shader.setVec3("spotLight.specular", 0.5f, 0.5f, 0.5f);        // 适当镜面反射       // 适当镜面反射

        shader.setFloat("spotLight.constant", 1.0f);
        shader.setFloat("spotLight.linear", 0.09f);
        shader.setFloat("spotLight.quadratic", 0.032f);
        shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));


        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
		shader.setVec3("dirLightDir", -0.2f, -1.0f, -0.3f);
        shader.setVec3("spotLightDir", camera.Front);

        // 渲染模型
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // 向下移动到屏幕底部中央位置
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// 缩放
        shader.setMat4("model", model);
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightPos", lightPos);
        ourModel.Draw(shader);

        // 设置等立方体着色器并传递变换矩阵
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        // 也绘制灯立方体
        glBindVertexArray(lightCubeVAO);

        model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPositions);
        model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
        lightCubeShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // 检查并调用事件，交换缓冲
        glfwSwapBuffers(window);
        glfwPollEvents();          // 处理事件
    }


    glfwTerminate(); // 终止GLFW库
    return 0;
}

// 窗口回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height); // glViewport 用于设置视口大小
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

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gammaKeyPressed)
    {
        gamma = !gamma;
        gammaKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
    {
        gammaKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.Sensitivity += 0.001f; // 增加鼠标灵敏度
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.Sensitivity -= 0.001f; // 减少鼠标灵敏度
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
}

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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// 从文件加载纹理
unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }

    return textureID;
}


