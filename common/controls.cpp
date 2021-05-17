#define _USE_MATH_DEFINES
// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"
#include <math.h>

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

//// Initial position : on +Z
//glm::vec3 position = glm::vec3( 0, 3.5, 11.5 ); 
//// Initial horizontal angle : toward -Z
//float horizontalAngle = 0.0f;
//// Initial vertical angle : none
//float verticalAngle = -M_PI / 2.0f;


// Initial position : on +Z
glm::vec3 position = glm::vec3(0, 1, 95);
// Initial horizontal angle : toward -Z
float horizontalAngle = M_PI;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 20.0f; // 3 units / second
float mouseSpeed = 0.002f;

double xpos, ypos;
double xpos0, ypos0;
bool isFirstmouse = true;

void computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	glfwGetCursorPos(window, &xpos, &ypos);

	// Compute new orientation
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
		if (!isFirstmouse) {
			if (xpos != xpos0) horizontalAngle += mouseSpeed * float(xpos - xpos0);
			if (ypos != ypos0) {
				verticalAngle -= mouseSpeed * float(ypos - ypos0);
				if (verticalAngle < -M_PI / 2.f)
					verticalAngle = -M_PI / 2.f;
				if (verticalAngle > M_PI / 2.f)
					verticalAngle = M_PI / 2.f;
			}
		}
		else
			isFirstmouse = false;

		xpos0 = xpos;
		ypos0 = ypos;
	}
	else // in the case of GLFW_RELEASE
		isFirstmouse = true;

	//// Direction : Spherical coordinates to Cartesian coordinates conversion
	//glm::vec3 direction(
	//	cos(verticalAngle) * sin(horizontalAngle),
	//	cos(verticalAngle) * cos(horizontalAngle),
	//	sin(verticalAngle)
	//);

	//// Right vector
	//glm::vec3 right = glm::vec3(
	//	sin(horizontalAngle - M_PI / 2.0f),
	//	cos(horizontalAngle - M_PI / 2.0f),
	//	0
	//);

	//glm::vec3 zaxis(0, 0, 1);

	//// Forward vector
	//glm::vec3 forward = glm::cross(zaxis, right);

	//// Up vector
	//glm::vec3 up = glm::cross(direction, right);

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - M_PI/2.0f), 
		0,
		cos(horizontalAngle - M_PI/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	// Strafe up
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		position += up * deltaTime * speed;
	}
	// Strafe down
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
		if (position.z < 99)
			position -= up * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
		position -= right * deltaTime * speed;
	}
	// Move forward (zoom in)
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		if (position.z > 3)
			position += direction * deltaTime * speed;
	}
	// Move backward (zoom out)
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		if (position.z < 99)
			position -= direction * deltaTime * speed;
	}

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	float aspectRatio = window_width / window_height;
	ProjectionMatrix = glm::perspective(FoV, aspectRatio, 0.1f, 100.0f);

	// Camera matrix
	ViewMatrix       = glm::lookAt(
								position,           // Camera is here
								position+direction, // and looks here : at the same position, plus "direction"
								up                  // Head is up (set to 0,-1,0 to look upside-down)
						   );

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}