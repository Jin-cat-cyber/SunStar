# 深空星海 — Deep Space Galaxy

基于 OpenGL 3.3 Core Profile 的实时 3D 太空场景渲染项目。

## 技术特性

- **GPU 实例化**：50,000 颗小行星，单次 Draw Call 渲染
- **三层恒星渲染**：核心 + 日冕（Billboarding）+ 辉光（加法混合）
- **四元数 FPS 相机**：万向节锁定免疫，WASD + 鼠标控制
- **Phong 光照模型**：定向光 + 点光源，模拟恒星辐射

## 依赖库

GLFW 3.x | GLAD | GLM | Assimp | stb_image

## 构建

Visual Studio 2022，打开 OpenGL_Learning.sln，编译运行。

## 操作

| 按键 | 功能 |
|------|------|
| W/A/S/D | 移动 |
| 鼠标 | 视角 |
| 滚轮 | 缩放 |
| TAB | 切换光标锁定 |
| ↑/↓ | 调节灵敏度 |
