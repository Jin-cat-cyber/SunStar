#include "Camera_Space.h"

// 带有向量参数的构造函数
Camera_Space::Camera_Space(glm::vec3 position) :Speed(SPEED), Sensitivity(SENSITIVITY), Fov(FOV)
{
	Position = position;
	Orient = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // 单位四元数：朝向 (0,0,-1)
	updateCameraVectors();
}

// 带有标量参数的构造函数
Camera_Space::Camera_Space(float posX, float posY, float posZ) : Speed(SPEED), Sensitivity(SENSITIVITY), Fov(FOV)
{
	Position = glm::vec3(posX, posY, posZ);
	Orient = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // 单位四元数：朝向 (0,0,-1)
	updateCameraVectors();
}

// 视图矩阵构造函数
glm::mat4 Camera_Space::GetViewMatrix()
{
	return glm::lookAt(Position, Position + Front, Up);
}

// 键盘控制函数
void Camera_Space::ProcessKeyboard(Camera_Movement direction, float deltaTime)
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
void Camera_Space::ProcessMouseMovement(float xoffset, float yoffset)
{
	xoffset *= Sensitivity;
	yoffset *= Sensitivity;

	// 计算当前的俯仰角-->弧度
	float YawAngle = glm::radians(-xoffset); // 计算当前俯仰角（以度为单位）
	float PitchAngle = glm::radians(yoffset);

	// 增量四元数：绕相机的局部Up轴旋转yaw
	glm::quat deltaYaw = glm::angleAxis(YawAngle, Up);
	glm::quat deltaPitch = glm::angleAxis(PitchAngle, Right); // 绕相机的局部Right轴旋转pitch

	Orient = deltaYaw * Orient * deltaPitch; // 注意乘法顺序，先应用pitch再应用yaw
	Orient = glm::normalize(Orient); // 归一化四元数，避免数值误差导致旋转失真

	// 重新计算方向向量
	updateCameraVectors();
}

// 鼠标滚轮控制函数
void Camera_Space::ProcessMouseScroll(float yoffset)
{
	Fov -= (float)yoffset;
	if (Fov < 1.0f)
		Fov = 1.0f;
	if (Fov > 45.0f)
		Fov = 45.0f;
}

// 相机状态更新函数
void Camera_Space::updateCameraVectors()
{
	Front = glm::normalize(Orient * glm::vec3(0.0f, 0.0f, -1.0f));
	Right = glm::normalize(Orient * glm::vec3(1.0f, 0.0f, 0.0f));
	Up = glm::normalize(Orient * glm::vec3(0.0f, 1.0f, 0.0f));
}