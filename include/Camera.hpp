#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum movement
{
	FORWARD,
	RIGHT,
	LEFT,
	BACKWARD	
};

class Camera
{
private:
	glm::vec3 pos;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right; // noramlized of 'cross(front * up)'
	const glm::vec3 world_up = glm::vec3(0, 1, 0);

	float yaw;
	float pitch;
	float mouse_sensitivity;
	float movement_speed;
	bool first_movement = true;

	double xPos = 0, yPos = 0;

public:
	const float speed = 2.0f;

	Camera(glm::vec3 pos = glm::vec3(0, 0, 0), \
		glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f), \
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), \
		glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f),
		float yaw = -90.0f, float pitch = 0.0f, \
		float mouse_sensitivity = 0.1f,
		float movement_speed = 10.0f)
		: pos(pos), front(front), up(up), right(right),
		yaw(yaw), pitch(pitch), \
		mouse_sensitivity(mouse_sensitivity), \
		movement_speed(movement_speed) 
		{};
	~Camera();

	void move(movement dir, float delta_time);
	void rotate(float xpos, float ypos);
	glm::mat4 getViewMatrix(void) const;
	glm::vec3 getPosition(void) const;
};

Camera::~Camera()
{
}

void Camera::move(movement dir, float delta_time)
{
	switch (dir)
	{
		case FORWARD:
			pos += front * movement_speed * delta_time;
			break;
		case BACKWARD:
			pos -= front * movement_speed * delta_time;
			break;
		case RIGHT:
			pos += right * movement_speed * delta_time;
			break;
		case LEFT:
			pos -= right * movement_speed * delta_time;
			break;
	}
}


void Camera::rotate(float xpos, float ypos)
{
	// TODO:
	if (first_movement) {
		xpos = xpos;
		ypos = ypos;
		first_movement = false;
	}
	float xoffset;
	float yoffset;
	xoffset = xpos - xPos;
	yoffset = ypos - yPos;
	xPos = xpos;
	yPos = ypos;
	xoffset *= mouse_sensitivity;
	yoffset *= mouse_sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = -sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);
	right = glm::normalize(glm::cross(front, world_up));
	up = glm::normalize(-glm::cross(front, right));
}

glm::mat4 Camera::getViewMatrix() const
{
	return (glm::lookAt(pos, pos + front, up));
}

glm::vec3 Camera::getPosition() const
{
	return (pos);
}

#endif
