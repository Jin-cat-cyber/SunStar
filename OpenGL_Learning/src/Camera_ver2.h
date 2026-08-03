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
const float SENSITIVITY = 0.07f;	// 灵敏度
const float FOV = 45.0f;

class Camera_ver2
{
public:
	glm::vec3 Position; // 位置
	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);    // 前方向，默认指向 -Z
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);       // 上方向
	glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);    // 右方向
	glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);  // 世界坐标系的上方向

	glm::quat Orient;	// 四元数表示的旋转
	float Speed = 2.5f; // 移动速度
	float Sensitivity = 0.07f; // 鼠标灵敏度
	float Fov = 45.0f; // 视野

	// 两种摄像机构造函数
	//  带有向量参数的
	Camera_ver2(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f));
	//  带有标量参数的
	Camera_ver2(float posX, float posY, float posZ);

	// 视图矩阵构造函数
	glm::mat4 GetViewMatrix();

	// 键盘控制函数
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	// 鼠标移动控制函数
	void ProcessMouseMovement(float xoffset, float yoffset);

	// 鼠标滚轮控制函数
	void ProcessMouseScroll(float yoffset);

private:
	// 根据相机（更新后的）欧拉角计算前向量
	void updateCameraVectors();
};

