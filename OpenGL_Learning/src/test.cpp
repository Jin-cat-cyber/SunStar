#include <glad/glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "BezierCurve.h"
#include "Shader.h"
#include <iostream>
#include "Model.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// ============ 全局 ============
const unsigned int W = 960, H = 700;
BezierCurve curve;
int dragIdx = -1;


// 屏幕坐标 → 世界坐标
glm::vec2 toWorld(double x, double y)
{
    float aspect = (float)W / H, half = 1.15f;
    return glm::vec2((float)(x / W * 2.0 - 1.0) * half * aspect,
        (float)(-y / H * 2.0 + 1.0) * half);
}

// ============ 回调 ============
void framebuffer_size_callback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

// 鼠标点击：左键按下时选中或添加控制点，右键按下时删除控制点
void mouse_button_callback(GLFWwindow* w, int btn, int act, int)
{
    double x, y; glfwGetCursorPos(w, &x, &y);
    glm::vec2 p = toWorld(x, y);
    if (btn == GLFW_MOUSE_BUTTON_LEFT && act == GLFW_PRESS)
    {
        int i = curve.findNearest(p);
        dragIdx = (i >= 0) ? i : (curve.addPoint(p), curve.count() - 1);
    }
    if (btn == GLFW_MOUSE_BUTTON_LEFT && act == GLFW_RELEASE) dragIdx = -1;
    if (btn == GLFW_MOUSE_BUTTON_RIGHT && act == GLFW_PRESS)
    {
        int i = curve.findNearest(p);
        if (i >= 0) curve.removePoint(i);
    }
}

void cursor_pos_callback(GLFWwindow* w, double x, double y)
{
    if (dragIdx >= 0) curve.movePoint(dragIdx, toWorld(x, y));
}

// ============ 画线 / 画点 ============
void drawLine(const std::vector<glm::vec2>& pts, Shader& s, float r, float g, float b, float w = 1.0f)
{
    if (pts.size() < 2) return;
    glLineWidth(w);
    s.setVec4("uColor", r, g, b, 1.0f);
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo);
    glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec2), pts.data(), GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)pts.size());
    glDeleteVertexArrays(1, &vao); glDeleteBuffers(1, &vbo);
    glLineWidth(1.0f);
}

void drawDot(glm::vec2 p, Shader& s, float r, float g, float b, float size = 10.0f)
{
    glPointSize(size);
    s.setVec4("uColor", r, g, b, 1.0f);
    unsigned int vao, vbo; float d[2] = { p.x, p.y };
    glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo);
    glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(d), d, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
    glDrawArrays(GL_POINTS, 0, 1);
    glDeleteVertexArrays(1, &vao); glDeleteBuffers(1, &vbo);
    glPointSize(1.0f);
}

// ============ 主函数 ============
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(W, H, "Bezier Curve", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    Shader shader("res/shader/test_ver.shader", "res/shader/test_frag.shader");

    // 预设控制点
    curve.addPoint(glm::vec2(-0.7f, -0.3f));
    curve.addPoint(glm::vec2(-0.3f, 0.6f));
    curve.addPoint(glm::vec2(0.3f, -0.5f));
    curve.addPoint(glm::vec2(0.7f, 0.3f));

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float aspect = (float)W / H;
        glm::mat4 proj = glm::ortho(-1.15f * aspect, 1.15f * aspect, -1.15f, 1.15f);
        shader.use();
        shader.setMat4("projection", proj);

        const auto& ctrl = curve.points();
        auto curvePts = curve.sample(200);

        // 控制多边形
        drawLine(ctrl, shader, 0.5f, 0.5f, 0.55f);
        // Bezier 曲线
        drawLine(curvePts, shader, 1.0f, 0.85f, 0.05f, 3.0f);
        // 控制点
        for (int i = 0; i < curve.count(); ++i)
        {
            float cr = 0.8f, cg = 0.8f, cb = 0.8f;
            if (i == 0) { cr = 0; cg = 0.8f; cb = 0; }
            if (i == curve.count() - 1) { cr = 0; cg = 0.4f; cb = 1; }
            if (i == dragIdx) { cr = 1; cg = 0.2f; cb = 0.2f; }
            drawDot(ctrl[i], shader, cr, cg, cb);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
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
