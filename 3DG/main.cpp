#define _CRT_SECURE_NO_WARNINGS 1
#define _USE_MATH_DEFINES
#define TEST
//#define EXECUTE

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
using namespace std;

#include <vector>
#include <math.h>
#include <algorithm>
#include <thread>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define MAX_POINTS 999

int window_height;
int window_width;
const float toRadians = M_PI / 180.0f;
float currentAngle = 0.0f;
float startx, starty, endx, endy;
bool isFirstDraw = true;
bool isStartEndSet = false;
bool isColorSet = false;
bool isLineSet = false;

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

void tConsole() {
	while (isFirstDraw) {
		// wait
	}
	cout << "Start point: ";
	cin >> startx >> starty;
	cout << "End point: ";
	cin >> endx >> endy;

	isStartEndSet = true;
}

int main(void)
{
	// 2D Points Generation
	cout << "Enter the coordinates. (range: -500 ~ +500)" << endl;
	cout << "Enter '*' to finish typing." << endl;
	
	float x, y;
	int cnt = 0;

#ifdef EXECUTE
	vector<Point> point;
	while (1) {
		cout << "Point[" << cnt << "]: ";
		cin >> x;
		if (getchar() == '*') {
			if (cnt >= 10) break;
			else {
				cout << "The number of points must be at least 10" << endl;
				cout << "(current num points: " << cnt << ")" << endl;
				cin.clear();
				cin.ignore(256, '\n');
				continue;
			}
		}
			
		cin >> y;
		if (getchar() == '*') {
			if (cnt >= 10) break;
			else {
				cout << "The number of points must be at least 10" << endl;
				cout << "(current num points: " << cnt << ")" << endl;
				cin.clear();
				cin.ignore(256, '\n');
				continue;
			}
		}
		
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
			point.push_back(Point(x, y));
			cnt++;
		}

	};
	cin.clear();
	cin.ignore(256, '\n');
	cout << "num points: " << cnt << endl << endl;
#endif // EXECUTE

#ifdef TEST
	Point point[20] = { {60,-8}, {500,500}, {300,300}, {0,70}, {40,30},
						{20,0}, {0,-310}, {70,0}, {-88,-80}, {-220,500},
						{304,-220}, {-20,-340}, {460,0}, {220,0}, {-10,20},
						{0,20}, {-70,80}, {-330,-440}, {140,-70}, {70,-500}, };
	cnt = 20;
	for (int i = 0; i < cnt; i++)
		cout << "Point[" << i << "]: " << point[i].x << " " << point[i].y << endl;
	cout << "Point[20]: *" << endl;
	cout << "num points: " << cnt << endl << endl;
#endif // TEST
	
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

	// Black background
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
	GLuint programID = LoadShaders();

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
	static GLfloat* g_color_points = new GLfloat[MAX_POINTS * 3];

	// Make vertices and color for lines
	static GLfloat* g_vertex_lines = new GLfloat[MAX_POINTS * 2 * 3];
	static GLfloat* g_color_lines = new GLfloat[MAX_POINTS * 2 * 3];

	// Generate& Bind Buffer
	GLuint vertexbuffer_points;
	glGenBuffers(1, &vertexbuffer_points);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_points);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_points), g_vertex_points, GL_STATIC_DRAW);

	GLuint colorbuffer_points;
	glGenBuffers(1, &colorbuffer_points);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer_points);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MAX_POINTS * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

	GLuint vertexbuffer_lines;
	glGenBuffers(1, &vertexbuffer_lines);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_lines);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MAX_POINTS * 2 * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

	GLuint colorbuffer_lines;
	glGenBuffers(1, &colorbuffer_lines);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer_lines);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MAX_POINTS * 2 * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

	int lineIdx[MAX_POINTS];
	int endidx;
	thread t(tConsole);

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
		
		// For rotate with mouse wheel
		MVP = glm::rotate(MVP, currentAngle * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));

		// Indicate the start and the end point
		if (!isFirstDraw && !isColorSet && isStartEndSet) {
			// Find the start point
			pair<float, int> distance[MAX_POINTS];
			for (int i = 0; i < cnt; i++) {
				distance[i].first = sqrt(pow(point[i].x - startx, 2) + pow(point[i].y - starty, 2));
				distance[i].second = i;
			}
			sort(distance, distance + cnt);
			int startidx = distance[0].second;

			// Store idx for drawing lines
			for (int i = 0; i < cnt; i++)
				lineIdx[i] = distance[i].second;

			// Find the end point
			for (int i = 0; i < cnt; i++) {
				distance[i].first = abs(point[i].x - endx) + abs(point[i].y - endy);
				distance[i].second = i;
			}
			sort(distance, distance + cnt);
			endidx = distance[0].second;
			if (endidx == startidx)
				endidx = distance[1].second;

			// Set the color of start and the end point
			point[startidx].r = 1.f;
			point[startidx].g = 0.f;
			point[startidx].b = 0.f;

			point[endidx].r = 0.f;
			point[endidx].g = 0.f;
			point[endidx].b = 1.f;

			isColorSet = true;
		}

		//  Draw lines
		if (isColorSet) {
			glLineWidth(10.f);
			int i = 0, j = 0, idx1, idx2;
			bool isSkip = false;

			while (i + 1 < cnt) {
				idx1 = lineIdx[i++];
				idx2 = lineIdx[i];
				if (idx2 == endidx && i + 1 != cnt) { // if endidx is in the middle, skip it
					idx2 = lineIdx[++i];
					isSkip = true;
				}
				g_vertex_lines[j++] = point[idx1].x;
				g_vertex_lines[j++] = point[idx1].y;
				g_vertex_lines[j++] = 0;
				g_vertex_lines[j++] = point[idx2].x;
				g_vertex_lines[j++] = point[idx2].y;
				g_vertex_lines[j++] = 0;
			}
			if (isSkip) {
				g_vertex_lines[j++] = point[idx2].x;
				g_vertex_lines[j++] = point[idx2].y;
				g_vertex_lines[j++] = 0;
				g_vertex_lines[j++] = point[endidx].x;
				g_vertex_lines[j++] = point[endidx].y;
				g_vertex_lines[j++] = 0;
			}

			j = 0;
			for (int i = 0; i < (cnt - 1) * 6; i++) {
				g_color_lines[j++] = 1.f;
			}

			if (!isLineSet) isLineSet = true;
		}

		// Fill GPU buffer for dynamic draw
		glPointSize(10.f);
		j = 0;
		for (int i = 0; i < cnt; i++) {
			g_color_points[j++] = point[i].r;
			g_color_points[j++] = point[i].g;
			g_color_points[j++] = point[i].b;
		}

		// Bind buffer for dynamic draw
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer_points);
		glBufferData(GL_ARRAY_BUFFER, MAX_POINTS * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_POINTS * 3 * sizeof(GLfloat), g_color_points);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_lines);
		glBufferData(GL_ARRAY_BUFFER, MAX_POINTS * 2 * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_POINTS * 2 * 3 * sizeof(GLfloat), g_vertex_lines);

		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer_lines);
		glBufferData(GL_ARRAY_BUFFER, MAX_POINTS * 2 * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_POINTS * 2 * 3 * sizeof(GLfloat), g_color_lines);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		
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

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		if (isLineSet) {
			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_lines);
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
			glBindBuffer(GL_ARRAY_BUFFER, colorbuffer_lines);
			glVertexAttribPointer(
				1,                          // attribute. No particular reason for 1, but must match the layout in the shader.
				3,                          // size
				GL_FLOAT,                   // type
				GL_FALSE,                   // normalized?
				0,                          // stride
				(void*)0                    // array buffer offset
			);

			glDrawArrays(GL_LINES, 0, (cnt - 1) * 2); 

			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(0);
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		if (isFirstDraw) {
			isFirstDraw = false;
		}

	} // Check if the ESC key or Q was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer_points);
	glDeleteBuffers(1, &colorbuffer_points);
	glDeleteBuffers(1, &vertexbuffer_lines);
	glDeleteBuffers(1, &colorbuffer_lines);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	// Join thread
	t.join();

	return 0;
}

