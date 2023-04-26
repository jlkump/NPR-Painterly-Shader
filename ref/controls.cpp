#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <GLFW/glfw3.h>

#include "controls.hpp"

extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

glm::mat4 ProjectionMatrix;
glm::mat4 ViewMatrix;

// position
glm::vec3 position = glm::vec3(0, 0, 5);
// horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// vertical angle : 0, look at the horizon
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 0.001f; // 3 units / second
float mouseSpeed = 0.001f;

void computeMatricesFromInputs() {
	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	// Reset mouse position for next frame
	int windowWidth, windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Compute new orientation
	horizontalAngle += mouseSpeed * deltaTime * float(windowWidth / 2 - xpos);
	verticalAngle += mouseSpeed * deltaTime * float(windowHeight / 2 - ypos);

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// Up vector : perpendicular to both direction and right
	glm::vec3 up = glm::cross(right, direction);

	// Move forward
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * deltaTime * speed;
	}

	float FoV = initialFoV; // - 5 * glfwGetMouseWheel();

	// Projection matrix : 45&deg; Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);
}




//// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
//glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)4 / (float)3, 0.1f, 100.0f);

//// Or, for an ortho camera :
//// glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

float height = 2.0f;
float radius = 5.0f;

void computeMatricesFromRotationOverTime(glm::vec3 center_of_rotation) {
	static double lastTime = glfwGetTime();
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Move up
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		height +=  deltaTime * speed;
	}
	// Move down
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		height -= deltaTime * speed;
	}

	// Move in
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		radius -= deltaTime * speed;
	}
	// Move out
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		radius += deltaTime * speed;
	}

	position = glm::vec3(center_of_rotation.x + radius * cos(deltaTime), height, center_of_rotation.z + radius * sin(deltaTime));
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	ProjectionMatrix = glm::perspective(glm::radians(initialFoV), 4.0f / 3.0f, 0.1f, 100.0f);
	ViewMatrix = glm::lookAt(position, center_of_rotation, up);
}

glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}