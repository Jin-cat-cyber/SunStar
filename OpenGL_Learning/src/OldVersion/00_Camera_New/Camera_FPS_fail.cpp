#include "Camera_FPS_fail.h"

// 带有向量参数的构造函数
Camera_FPS::Camera_FPS(glm::vec3 position, float yaw, float pitch) : Speed(SPEED), Sensitivity(SENSITIVITY), Fov(FOV)
{
	Position = position;
	Yaw = yaw;
	Pitch = pitch;
	Orient = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // 单位四元数：朝向 (0,0,-1)
	updateCameraVectors();
}

// 带有标量参数的构造函数
Camera_FPS::Camera_FPS(float posX, float posY, float posZ, float yaw, float pitch) : Speed(SPEED), Sensitivity(SENSITIVITY), Fov(FOV)
{
	Position = glm::vec3(posX, posY, posZ);
	Yaw = yaw;
	Pitch = pitch;
	Orient = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // 单位四元数：朝向 (0,0,-1)
	updateCameraVectors();
}

// 视图矩阵构造函数
glm::mat4 Camera_FPS::GetViewMatrix()
{
	return glm::lookAt(Position, Position + Front, Up);
}

// 键盘控制函数
void Camera_FPS::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = Speed * deltaTime; // 刷新率
	if (direction == FORWARD)
		Position += Front * velocity;
	if (direction == BACKWARD)
		Position -= Front * velocity;
	if (direction == LEFT)
		Position -= Right * velocity;
	if (direction == RIGHT)
		Position += Right * velocity;

	// 普通FPS相机添加
	//Position.y = 0.0f; // 取消Y轴移动,确保玩家只能在xz上移动
}

// 鼠标移动控制函数，完全基于四元数旋转实现
void Camera_FPS::ProcessMouseMovement(float xoffset, float yoffset)
{
	xoffset *= Sensitivity;
	yoffset *= Sensitivity;

	Yaw -= glm::radians(xoffset);   // 注意方向
	Pitch += glm::radians(yoffset);
	// 限制俯仰角
	if (Pitch > glm::radians(89.0f))  Pitch = glm::radians(89.0f);
	if (Pitch < glm::radians(-89.0f)) Pitch = glm::radians(-89.0f);
	// 用新角度重新合成四元数
	Orient = glm::angleAxis(Yaw, WorldUp) * glm::angleAxis(Pitch, glm::vec3(1, 0, 0));
	Orient = glm::normalize(Orient);

	// 重新计算方向向量
	updateCameraVectors();
}

// 鼠标滚轮控制函数
void Camera_FPS::ProcessMouseScroll(float yoffset)
{
	Fov -= (float)yoffset;
	if (Fov < 1.0f)
		Fov = 1.0f;
	if (Fov > 45.0f)
		Fov = 45.0f;
}

// 相机状态更新函数
void Camera_FPS::updateCameraVectors()
{
	Front = glm::normalize(Orient * glm::vec3(0.0f, 0.0f, -1.0f));
	Right = glm::normalize(Orient * glm::vec3(1.0f, 0.0f, 0.0f));
	Up = glm::normalize(Orient * glm::vec3(0.0f, 1.0f, 0.0f));
}