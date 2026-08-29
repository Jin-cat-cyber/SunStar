#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

// »˝÷÷ ”Õºæÿ’Û
enum ViewMode
{
	MODE_FREE	= 1,	// free camera
	MODE_REMOTE = 2,	// remote view: camera fixed, control the ship
	MODE_FOLLOW = 3		// third-person follow
};

// Spaceship: position + heading + basic movement
struct Spaceship
{
	glm::vec3 position = glm::vec3(-100.0f, 40.0f, 60.0f);

	// Heading quaternion. Initial value = the old hardcoded orientation
	// (rotate -90 X, 15 Y, 185 Z), so the ship starts facing the same way.
	glm::quat heading =
		glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::angleAxis(glm::radians(15.0f),  glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::angleAxis(glm::radians(185.0f), glm::vec3(0.0f, 0.0f, 1.0f));


	float speed = 0.0f;			// current speed (scalar, along nose)
	float targetSpeed = 0.0f;   // target speed, set by input
	float forwardMax = 30.0f;	// forward speed threshold
	float backwardMax = 10.0f;  // backward speed threshold (smaller)
	float accel = 20.0f;		// forward acceleration a
	float decel = 30.0f;		// deceleration (when released)
	float backwardAccel = 8.0f;   // backward acceleration a1 (|a1| < a)
	float verticalSpeed = 5.0f;		// vertical movement speed (slow, Z/X keys)
	float turnRate		= 10.0f;	// turn angular speed factor





	// Nose direction in world space (after fix-up, nose points to -Z)
	glm::vec3 Forward() const
	{
		return heading * glm::vec3(0.0f, -1.0f, 0.0f);
	}


	// Assmble model matrix, Equivalent to the old
	// translate / rotate / rotate / rotate / scale chain.
	glm::mat4 GetModelMatrix() const
	{
		glm::mat4 m = glm::mat4(1.0f);
		m = glm::translate(m, position);
		m = m * glm::mat4_cast(heading);
		m = glm::scale(m, glm::vec3(0.0005f));
		return m;
	}



	// Per-frame: speed eases toward targetSpeed (uniform accel), then move along nose
	void Update(float dt)
	{
		float rate;
		if (targetSpeed > 0.0f)		 rate = accel;			// forward
		else if (targetSpeed < 0.0f) rate = backwardAccel;	// backward
		else						 rate = decel;			// released, slow down

		float step = rate * dt;
		if (speed < targetSpeed)
			speed = glm::min(speed + step, targetSpeed);
		else if (speed > targetSpeed)
			speed = glm::max(speed - step, targetSpeed);

		position += Forward() * speed * dt;

	}

	// Yaw around the world Y axis (left/right turn)
	void Yaw(float angleDeg, float dt)
	{
		float a = glm::radians(angleDeg) * dt * turnRate;
		glm::quat d = glm::angleAxis(a, glm::vec3(0.0f, 1.0f, 0.0f));
		heading = glm::normalize(d * heading);
	}

	// Pitch around the world X axis (up/down)
	void Pitch(float angleDeg, float dt)
	{
		float a = glm::radians(angleDeg) * dt * turnRate;
		glm::quat d = glm::angleAxis(a, glm::vec3(1.0f, 0.0f, 0.0f));
		heading = glm::normalize(d * heading);
	}

	// Roll around the nose axis (local -Y), Q/E
	void Roll(float angleDeg, float dt)
	{
		float a = glm::radians(angleDeg) * dt * turnRate;
		glm::quat d = glm::angleAxis(a, glm::vec3(0.0f, -1.0f, 0.0f));
		heading = glm::normalize(heading * d);	  // right-multiply = local nose axis
	}

	// Move vertically along world Y (Z/X keys), slow, no accel
	void MoveUp(float dt) { position += glm::vec3(0.0f, 1.0f, 0.0f) * verticalSpeed * dt; }
	void MoveDown(float dt) { position -= glm::vec3(0.0f, 1.0f, 0.0f) * verticalSpeed * dt; }
};