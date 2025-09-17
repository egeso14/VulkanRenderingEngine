#include "keyboard_movement_controller.h"
#include <limits>

void VTA::KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, VTAGameObject& gameObject)
{
	glm::vec3 rotate{ 0 };

	if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
	if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
	if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
	if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

	if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) // not a good idea to compare a float value directly with 0
	{
		gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
	}
	
	//clamp pitch
	gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
	gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

	float yaw = gameObject.transform.rotation.y;
	const glm::vec3 forwardDirection{ sin(yaw), 0.f, cos(yaw) };
	const glm::vec3 rightDirection{ forwardDirection.z, 0.f, -forwardDirection.x };
	const glm::vec3 upDirection{ 0.f, -1.f, 0.f };

	glm::vec3 moveDir{ 0.f };

	if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDirection;
	if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDirection;
	if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDirection;
	if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDirection;
	if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDirection;
	if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDirection;

	if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) // not a good idea to compare a float value directly with 0
	{
		gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
	}
	
}
