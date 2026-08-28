#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
};

const float SPEED = 2.5f;			// 速度
const float SENSITIVITY = 0.07f;	// 鼠标灵敏度
const float FOV = 45.0f;			// 视场角

class Camera_ver2
{
public:
	glm::vec3 Position; // 位置
	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);    // 前向，默认为 -Z
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);       // 向上方向
	glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);    // 向右方向
	glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);  // 世界坐标系中的上方向

	glm::quat Orient;	// 使用四元数表示旋转
	float Speed = 2.5f; // 移动速度
	float Sensitivity = 0.07f; // 鼠标灵敏度
	float Fov = 45.0f; // 视场角

	// 构造函数：接受位置向量，带默认位置
	Camera_ver2(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f));
	// 构造函数：接受 x, y, z 分量
	Camera_ver2(float posX, float posY, float posZ);

	// 返回视图矩阵
	glm::mat4 GetViewMatrix();

	// 键盘控制
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	// 鼠标移动控制
	void ProcessMouseMovement(float xoffset, float yoffset);

	// 鼠标滚轮控制
	void ProcessMouseScroll(float yoffset);

	// 键盘旋转
	// 通过键盘绕世界 Y 轴旋转相机（Q/E），angleDeg 为带符号角度，deltaTime 用于平滑
	void ProcessKeyboardRotate(float angleDeg, float deltaTime);
	// 键盘俯仰
	// 通过键盘绕世界 X 轴旋转相机（R/F），angleDeg 为带符号角度
	void ProcessKeyboardPitch(float angleDeg, float deltaTime);

private:
	// 根据四元数重新计算 Front/Up/Right 向量
	void updateCameraVectors();
};
