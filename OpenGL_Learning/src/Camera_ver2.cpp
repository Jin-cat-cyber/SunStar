#include "Camera_ver2.h"

// 构造函数：接受位置向量
Camera_ver2::Camera_ver2(glm::vec3 position) :Speed(SPEED), Sensitivity(SENSITIVITY), Fov(FOV)
{
	Position = position;
	Orient = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // 单位四元数，表示朝向 (0,0,-1)
	updateCameraVectors();
}

// 构造函数：接受 x, y, z 分量
Camera_ver2::Camera_ver2(float posX, float posY, float posZ) : Speed(SPEED), Sensitivity(SENSITIVITY), Fov(FOV)
{
	Position = glm::vec3(posX, posY, posZ);
	Orient = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // 单位四元数，表示朝向 (0,0,-1)
	updateCameraVectors();
}

// 返回视图矩阵
glm::mat4 Camera_ver2::GetViewMatrix()
{
	return glm::lookAt(Position, Position + Front, Up);
}

// 键盘控制
void Camera_ver2::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = Speed * deltaTime * 10; // 速度
	if (direction == FORWARD)
		Position += Front * velocity;
	if (direction == BACKWARD)
		Position -= Front * velocity;
	if (direction == LEFT)
		Position -= Right * velocity;
	if (direction == RIGHT)
		Position += Right * velocity;

	// 类似 FPS 的移动
	//Position.y = 0.0f; // 锁定 Y 轴，使相机仅在 xz 平面上移动
}

// 鼠标移动控制，使用完整四元数旋转
void Camera_ver2::ProcessMouseMovement(float xoffset, float yoffset)
{
	xoffset *= Sensitivity;
	yoffset *= Sensitivity;

	// 计算俯仰/偏航角
	float YawAngle = glm::radians(-xoffset); // 将偏移转换为弧度
	float PitchAngle = glm::radians(yoffset);

	// 绕局部 Up 轴旋转偏航
	//glm::quat deltaYaw = glm::angleAxis(YawAngle, Up);
	//glm::quat deltaPitch = glm::angleAxis(PitchAngle, Right); // 绕局部 Right 轴旋转俯仰

	// 绕世界轴旋转（父空间 = 世界空间），包括俯仰和偏航
	// 绕世界 X 轴（Right）做俯仰，绕世界 Y 轴（Up）做偏航；使用世界轴可保持地平线稳定
	glm::quat deltaYaw = glm::angleAxis(YawAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // 绕世界 Y 轴
	glm::quat deltaPitch = glm::angleAxis(PitchAngle, glm::vec3(1.0f, 0.0f, 0.0f)); // 绕世界 X 轴
	Orient = Orient * deltaPitch * deltaYaw; // 顺序重要：先俯仰再偏航
	Orient = glm::normalize(Orient); // 归一化以避免漂移

	// 重新计算方向向量
	updateCameraVectors();
}

// 鼠标滚轮控制
void Camera_ver2::ProcessMouseScroll(float yoffset)
{
	Fov -= (float)yoffset;
	if (Fov < 1.0f)
		Fov = 1.0f;
	if (Fov > 45.0f)
		Fov = 45.0f;
}

void Camera_ver2::ProcessKeyboardRotate(float angleDeg, float deltaTime)
{
	float angle = glm::radians(angleDeg) * deltaTime * 50.0f;   // 旋转速度因子，可调整
	glm::quat delta = glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f)); // 绕世界 Y 轴
	Orient = delta * Orient;   // 左乘 = 世界空间旋转
	Orient = glm::normalize(Orient);
	updateCameraVectors();
}

// 通过键盘绕世界 X 轴旋转相机（R/F），用于俯仰
void Camera_ver2::ProcessKeyboardPitch(float angleDeg, float deltaTime)
{
	float pitchAngle = glm::radians(angleDeg) * deltaTime * 50.0f;
	glm::quat deltaP = glm::angleAxis(pitchAngle, glm::vec3(1.0f, 0.0f, 0.0f)); // 世界 X 轴
	Orient = Orient * deltaP;   // 右乘，与鼠标俯仰保持一致
	Orient = glm::normalize(Orient);
	updateCameraVectors();
}

// 更新相机方向向量
void Camera_ver2::updateCameraVectors()
{
	Front = glm::normalize(Orient * glm::vec3(0.0f, 0.0f, -1.0f));
	Right = glm::normalize(Orient * glm::vec3(1.0f, 0.0f, 0.0f));
	Up = glm::normalize(Orient * glm::vec3(0.0f, 1.0f, 0.0f));
}
