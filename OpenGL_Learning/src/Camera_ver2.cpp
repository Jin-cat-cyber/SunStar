#include "Camera_ver2.h"

// �������������Ĺ��캯��
Camera_ver2::Camera_ver2(glm::vec3 position) :Speed(SPEED), Sensitivity(SENSITIVITY), Fov(FOV)
{
	Position = position;
	Orient = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // ��λ��Ԫ�������� (0,0,-1)
	updateCameraVectors();
}

// ���б��������Ĺ��캯��
Camera_ver2::Camera_ver2(float posX, float posY, float posZ) : Speed(SPEED), Sensitivity(SENSITIVITY), Fov(FOV)
{
	Position = glm::vec3(posX, posY, posZ);
	Orient = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // ��λ��Ԫ�������� (0,0,-1)
	updateCameraVectors();
}

// ��ͼ�����캯��
glm::mat4 Camera_ver2::GetViewMatrix()
{
	return glm::lookAt(Position, Position + Front, Up);
}

// ���̿��ƺ���
void Camera_ver2::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = Speed * deltaTime * 10; // ˢ����
	if (direction == FORWARD)
		Position += Front * velocity;
	if (direction == BACKWARD)
		Position -= Front * velocity;
	if (direction == LEFT)
		Position -= Right * velocity;
	if (direction == RIGHT)
		Position += Right * velocity;

	// ��ͨFPS�������
	//Position.y = 0.0f; // ȡ��Y���ƶ�,ȷ�����ֻ����xz���ƶ�
}

// ����ƶ����ƺ�������ȫ������Ԫ����תʵ��
void Camera_ver2::ProcessMouseMovement(float xoffset, float yoffset)
{
	xoffset *= Sensitivity;
	yoffset *= Sensitivity;

	// ���㵱ǰ�ĸ�����-->����
	float YawAngle = glm::radians(-xoffset); // ���㵱ǰ�����ǣ��Զ�Ϊ��λ��
	float PitchAngle = glm::radians(yoffset);

	// ������Ԫ����������ľֲ�Up����תyaw
	//glm::quat deltaYaw = glm::angleAxis(YawAngle, Up);
	//glm::quat deltaPitch = glm::angleAxis(PitchAngle, Right); // ������ľֲ�Right����תpitch

	// ������Ԫ�����������������ת���ҳ� = ���ؿռ䣩����pitch��yaw��
	// pitch�Ʊ���X(����)��yaw�Ʊ���Y(����)��ȫ���ҳ�ȷ��ʼ��������������ת
	glm::quat deltaYaw = glm::angleAxis(YawAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // ����Y��
	glm::quat deltaPitch = glm::angleAxis(PitchAngle, glm::vec3(1.0f, 0.0f, 0.0f)); // ����X��
	Orient = Orient * deltaPitch * deltaYaw; // ע��˷�˳����Ӧ��pitch��Ӧ��yaw
	Orient = glm::normalize(Orient); // ��һ����Ԫ����������ֵ������תʧ��

	// ���¼��㷽������
	updateCameraVectors();
}

// �����ֿ��ƺ���
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
	float angle = glm::radians(angleDeg) * deltaTime * 50.0f;   // ת��ϵ�����ɵ�
	glm::quat delta = glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f)); // ������Y��
	Orient = delta * Orient;   // ���=��������
	Orient = glm::normalize(Orient);
	updateCameraVectors();
}

// rotate camera around world X axis by keyboard (R/F), pitch
void Camera_ver2::ProcessKeyboardPitch(float angleDeg, float deltaTime)
{
	float pitchAngle = glm::radians(angleDeg) * deltaTime * 50.0f;
	glm::quat deltaP = glm::angleAxis(pitchAngle, glm::vec3(1.0f, 0.0f, 0.0f)); // world X
	Orient = Orient * deltaP;   // right-multiply, consistent with mouse pitch
	Orient = glm::normalize(Orient);
	updateCameraVectors();
}

// ���״̬���º���
void Camera_ver2::updateCameraVectors()
{
	Front = glm::normalize(Orient * glm::vec3(0.0f, 0.0f, -1.0f));
	Right = glm::normalize(Orient * glm::vec3(1.0f, 0.0f, 0.0f));
	Up = glm::normalize(Orient * glm::vec3(0.0f, 1.0f, 0.0f));
}