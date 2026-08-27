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

const float SPEED = 2.5f;			// �ٶ�
const float SENSITIVITY = 0.07f;	// ������
const float FOV = 45.0f;

class Camera_ver2
{
public:
	glm::vec3 Position; // λ��
	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);    // ǰ����Ĭ��ָ�� -Z
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);       // �Ϸ���
	glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);    // �ҷ���
	glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);  // ��������ϵ���Ϸ���

	glm::quat Orient;	// ��Ԫ����ʾ����ת
	float Speed = 2.5f; // �ƶ��ٶ�
	float Sensitivity = 0.07f; // ���������
	float Fov = 45.0f; // ��Ұ

	// ������������캯��
	//  ��������������
	Camera_ver2(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f));
	//  ���б���������
	Camera_ver2(float posX, float posY, float posZ);

	// ��ͼ�����캯��
	glm::mat4 GetViewMatrix();

	// ���̿��ƺ���
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	// ����ƶ����ƺ���
	void ProcessMouseMovement(float xoffset, float yoffset);

	// �����ֿ��ƺ���
	void ProcessMouseScroll(float yoffset);

	// �����ת����
	// rotate camera around world Y axis by keyboard (Q/E), angleDeg signed, dt for smoothness
	void ProcessKeyboardRotate(float angleDeg, float deltaTime);
	// rotate camera around world X axis by keyboard (R/F), pitch, angleDeg signed
	void ProcessKeyboardPitch(float angleDeg, float deltaTime);

private:
	// ������������º�ģ�ŷ���Ǽ���ǰ����
	void updateCameraVectors();
};

