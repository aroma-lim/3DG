#define _CRT_SECURE_NO_WARNINGS 1

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/controls.hpp>

#include <iostream>
#include <vector>

using namespace std;

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define MAX_POINTS 999

int window_height;
int window_width;

class Point {
	public:
		float x; float y;
		float r; float g; float b;

		Point(float x, float y) {
			this->x = x;
			this->y = y;
			this->r = 1.f;
			this->g = 0.86f;
			this->b = 0.f;
		}
};


int main(void)
{
	// ==============================
	// 2D Points Generation
	// ============================== 
	cout << "Enter the coordinates. (range: -500 ~ +500)" << endl;
	cout << "Enter '*' to finish typing." << endl;

	vector<Point> point;
	float x, y;
	int cnt = 0;
	while (1) {
		cout << "Point[" << cnt << "]: ";
		cin >> x;
		if (getchar() == '*')
			break;
		cin >> y;
		if (getchar() == '*')
			break;
		
		try {
			if (cin.fail())
				throw 'c';
			else if (x < -500 || x > 500 || y < -500 || y > 500)
				throw out_of_range("(x, y) must be between -500 to 500");
			else
				throw 1;
		}
		catch (out_of_range & e) {
			cout << "Out of range: " << e.what() << endl;
		}
		catch (char c) {
			cin.clear();
			cin.ignore(256, '\n');
			cout << "The type must be integer" << endl;
		}
		catch (int i) {
			x /= 10;
			y /= 10;
			point.push_back(Point(x, y));
			cnt++;
		}

	};
	cout << "num points: " << cnt << endl;
	
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3DG", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Make vertices with entered points.
	static GLfloat g_vertex_points[MAX_POINTS * 3];
	int j = 0;
	for (int i = 0; i < cnt; i++) {
		g_vertex_points[j++] = point[i].x;
		g_vertex_points[j++] = point[i].y;
		g_vertex_points[j++] = 0;
	}

	// Make color for each vertex.
	static GLfloat g_color_points[MAX_POINTS * 3];

	// Make the first and the last point colors different
	point[0].r = 1.f;
	point[0].g = 0.f;
	point[0].b = 0.f;

	int last = cnt - 1;
	point[last].r = 0.f;
	point[last].g = 0.f;
	point[last].b = 1.f;

	j = 0;
	for (int i = 0; i < cnt; i++) {
		g_color_points[j++] = point[i].r;
		g_color_points[j++] = point[i].g;
		g_color_points[j++] = point[i].b;
	}

	// GPU buffer for axes
	static const GLfloat g_axes_vertex_data[] = {
		8, 0, 0, 200, 0, 0,
		0, 0, 0, 0, 200, 0,
		0, 0, 0, 0, 0, 200
	};
	static const GLfloat g_axes_color_data[] = {
		1, 0, 0, 1, 0, 0,
		0, 1, 0, 0, 1, 0,
		0, 0, 1, 0, 0, 1
	};

	GLuint vertexbuffer_axes;
	glGenBuffers(1, &vertexbuffer_axes);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_axes_vertex_data), g_axes_vertex_data, GL_STATIC_DRAW);

	GLuint colorbuffer_axes;
	glGenBuffers(1, &colorbuffer_axes);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_axes_color_data), g_axes_color_data, GL_STATIC_DRAW);

	GLuint vertexbuffer_points;
	glGenBuffers(1, &vertexbuffer_points);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_points);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_points), g_vertex_points, GL_STATIC_DRAW);

	GLuint colorbuffer_points;
	glGenBuffers(1, &colorbuffer_points);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer_points);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_points), g_color_points, GL_STATIC_DRAW);

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwGetWindowSize(window, &window_width, &window_height);

		glViewport(0, 0, window_width, window_height);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		
		glPointSize(10.f);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_points);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer_points);
		glVertexAttribPointer(
			1,                          // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                          // size
			GL_FLOAT,                   // type
			GL_FALSE,                   // normalized?
			0,                          // stride
			(void*)0                    // array buffer offset
		);

		glDrawArrays(GL_POINTS, 0, cnt);
		//glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_axes);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer_axes);
		glVertexAttribPointer(
			1,                          // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                          // size
			GL_FLOAT,                   // type
			GL_FALSE,                   // normalized?
			0,                          // stride
			(void*)0                    // array buffer offset
		);

		glDrawArrays(GL_LINES, 0, 3 * 2); // 3 lines * 2 vertex

		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer_points);
	glDeleteBuffers(1, &colorbuffer_points);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

