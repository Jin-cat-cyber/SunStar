#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

const float YAW = -90.0f;			// 偏航角
const float PITCH = 0.0f;			// 俯仰角
const float SPEED = 2.5f;			// 速度
const float SENSITIVITY = 0.07f;	// 灵敏度
const float FOV = 45.0f;

class Camera
{
public:
	glm::vec3 Position; // 位置
	glm::vec3 Front;    // 前方向
	glm::vec3 Up;       // 上方向
	glm::vec3 Right;    // 右方向
	glm::vec3 WorldUp;  // 世界坐标系的上方向

    float Yaw = -90.0f;	// 偏航角
    float Pitch = 0.0f;	// 俯仰角
	float Speed = 2.5f; // 移动速度
	float Sensitivity = 0.07f; // 鼠标灵敏度
    float Fov = 45.0f; // 视野

	// 两种摄像机构造函数
	//  带有向量参数的
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
		float yaw = YAW, float pitch = PITCH);
	//  带有标量参数的
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	// 视图矩阵构造函数
	glm::mat4 GetViewMatrix();

	// 键盘控制函数
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	// 鼠标移动控制函数
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

	// 鼠标滚轮控制函数
	void ProcessMouseScroll(float yoffset);

private:
	// 根据相机（更新后的）欧拉角计算前向量
	void updateCameraVectors();
};
