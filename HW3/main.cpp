#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

#define M_PI 3.141592654f
unsigned int g_windowWidth = 800;
unsigned int g_windowHeight = 600;
char* g_windowName = "HW3-3D-Basics";

GLFWwindow* g_window;

// Model data
std::vector<float> g_meshVertices;
std::vector<float> g_meshNormals;
std::vector<unsigned int> g_meshIndices;
GLfloat g_modelViewMatrix[16];

// Default options
bool enablePersp = true;
bool teapotSpin = false;
bool enableDolly = false;
bool showCheckerboard = false;

// Dolly zoom options
float distance = 4.5f;
float fov = M_PI / 4.f;
float changedFOV = fov;
float halfWidth = distance * std::tan(changedFOV / 2);

// Auxiliary math functions
float dotProduct(const float* a, const float* b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void computeVector(float * a, float * b, float * c, float * vectorA, float * vectorB) {
	vectorA[0] = b[0] - a[0];
	vectorA[1] = b[1] - a[1];
	vectorA[2] = b[2] - a[2];
	//
	vectorB[0] = c[0] - a[0];
	vectorB[1] = c[1] - a[1];
	vectorB[2] = c[2] - a[2];
}

void addVertNorm(float * n, int index) {
	g_meshNormals[index] += n[0];
	g_meshNormals[index + 1] += n[1];
	g_meshNormals[index + 2] += n[2];
}
void crossProduct(const float* a, const float* b, float* r)
{
	r[0] = a[1] * b[2] - a[2] * b[1];
	r[1] = a[2] * b[0] - a[0] * b[2];
	r[2] = a[0] * b[1] - a[1] * b[0];
}

float radianToDegree(float angleInRadian) {
	return angleInRadian * 180.f / M_PI;
}

void normalize(float* a)
{
	const float len = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);

	a[0] /= len;
	a[1] /= len;
	a[2] /= len;
}

void computeNormals()
{
	g_meshNormals.resize(g_meshVertices.size());

	// TASK 1
	//interate one triagle at a time.
	for (size_t i = 0; i < g_meshIndices.size(); i++) {
		//temp vectors for cross product
		std::vector<float> A;
		std::vector<float> B;
		std::vector<float> norm;
		A.resize(3);
		B.resize(3);
		norm.resize(3);
		float * vectorA = &A[0];
		float * vectorB = &B[0];
		//get index of edge
		unsigned int index = g_meshIndices[i];
		//get all three verticies of the triangle. Also walk the indecies vector.
		float * vertA = &g_meshVertices[3 * index];
		unsigned int index2 = g_meshIndices[++i];
		float * vertB = &g_meshVertices[3 * index2];
		unsigned int index3 = g_meshIndices[++i];
		float * vertC = &g_meshVertices[3 * index3];
		//get two vectors from a, b, c.
		computeVector(vertA, vertB, vertC, vectorA, vectorB);
		//compute the normal of the triangle. 
		float * n = &norm[0];
		crossProduct(vectorA, vectorB, n);
		//accumulate triagle face normals.
		addVertNorm(n, 3 * index); // vertex 1
		addVertNorm(n, 3 * index2); // vertex 2
		addVertNorm(n, 3 * index3); // vertex 3
	}
	//normalize
	for (int i = 0; i < g_meshNormals.size(); i+=3) {
		normalize(&g_meshNormals[i]);
	}
}

void loadObj(std::string p_path)
{
	std::ifstream nfile;
	nfile.open(p_path);
	std::string s;

	while (nfile >> s)
	{
		if (s.compare("v") == 0)
		{
			float x, y, z;
			nfile >> x >> y >> z;
			g_meshVertices.push_back(x);
			g_meshVertices.push_back(y);
			g_meshVertices.push_back(z);
		}		
		else if (s.compare("f") == 0)
		{
			std::string sa, sb, sc;
			unsigned int a, b, c;
			nfile >> sa >> sb >> sc;

			a = std::stoi(sa);
			b = std::stoi(sb);
			c = std::stoi(sc);

			g_meshIndices.push_back(a - 1);
			g_meshIndices.push_back(b - 1);
			g_meshIndices.push_back(c - 1);
		}
		else
		{
			std::getline(nfile, s);
		}
	}

	computeNormals();

	std::cout << p_path << " loaded. Vertices: " << g_meshVertices.size() / 3 << " Triangles: " << g_meshIndices.size() / 3 << std::endl;
}

double getTime()
{
	return glfwGetTime();
}
void setTime(float time) 
{
	glfwSetTime(time);
}

void glfwErrorCallback(int error, const char* description)
{
	std::cerr << "GLFW Error " << error << ": " << description << std::endl;
	exit(1);
}

void togglePerspective() {

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Perspective Projection
	if (enablePersp)
	{
		// Dolly zoom computation
		if (enableDolly) {
			// TASK 3
			//update FOV (in radians)
			changedFOV -= getTime()/5000;
			//update Distance
			distance = halfWidth / std::tan(changedFOV / 2);
			//convert to degrees.
			float changedFOVInDrgree = radianToDegree(changedFOV);
			if (distance <= 50 && distance >= 0) {
				gluPerspective(changedFOVInDrgree, (GLfloat)g_windowWidth / (GLfloat)g_windowHeight, 1.0f, 50.f);
				std::cout << "change in distance: " << distance << "\n" << "change in FOV: " << changedFOVInDrgree << std::endl;
			}
			else {
				changedFOV = fov;
				distance = 4.5f;
				setTime(0.0f);
			}
		}
		else {
		distance = 4.5f;
		float fovInDegree = radianToDegree(fov);
		gluPerspective(fovInDegree, (GLfloat)g_windowWidth / (GLfloat)g_windowHeight, 1.0f, 40.f);
		}
	}
	// Othogonal Projection
	else
	{
		// Scale down the object for a better view in orthographic projection
		//I changed the scale, because that just looked better to me.
		glScalef(1.0f, 1.0f, 1.0f);

		// TASK 3
		glOrtho(-4.0f, 4.0f, -3.0f, 3.0f, 1.0f, 40.0f);
	}
}

void glfwKeyCallback(GLFWwindow* p_window, int p_key, int p_scancode, int p_action, int p_mods)
{
	if (p_key == GLFW_KEY_ESCAPE && p_action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(g_window, GL_TRUE);
	}
	if (p_key == GLFW_KEY_P && p_action == GLFW_PRESS) {

		// Perspective Projection
		enablePersp = true;
		togglePerspective();
		std::cout << "Perspective activated\n";

	}
	if (p_key == GLFW_KEY_O && p_action == GLFW_PRESS) {

		// Orthographic Projection
		enablePersp = false;
		togglePerspective();
		std::cout << "Orthogonal activated\n";

	}
	if (p_key == GLFW_KEY_S && p_action == GLFW_PRESS) {

		// Toggle Spinning
		if (!teapotSpin) {
			std::cout << "Teapot spinning on\n";
		}
		else {
			std::cout << "Teapot spinning off\n";
		}
		teapotSpin = !teapotSpin;
	}
	if (p_key == GLFW_KEY_D && p_action == GLFW_PRESS) {

		// Toggle dolly zoom
		if (!enableDolly)
		{
			std::cout << "Dolly zoom on\n";
		}
		else {
			std::cout << "Dolly zoom off\n";
		}
		enableDolly = !enableDolly;
	}
	if (p_key == GLFW_KEY_C && p_action == GLFW_PRESS) {

		// Show/hide Checkerboard
		if (!showCheckerboard)
		{
			std::cout << "Show checkerboard\n";
		}
		else {
			std::cout << "Hide checkerboard\n";
		}
		showCheckerboard = !showCheckerboard;
	}
}

void initWindow()
{
	// initialize GLFW
	glfwSetErrorCallback(glfwErrorCallback);
	if (!glfwInit())
	{
		std::cerr << "GLFW Error: Could not initialize GLFW library" << std::endl;
		exit(1);
	}

	g_window = glfwCreateWindow(g_windowWidth, g_windowHeight, g_windowName, NULL, NULL);
	if (!g_window)
	{
		glfwTerminate();
		std::cerr << "GLFW Error: Could not initialize window" << std::endl;
		exit(1);
	}

	// callbacks
	glfwSetKeyCallback(g_window, glfwKeyCallback);

	// Make the window's context current
	glfwMakeContextCurrent(g_window);

	// turn on VSYNC
	glfwSwapInterval(1);
}

void initGL()
{
	glClearColor(1.f, 1.f, 1.f, 1.0f);
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
}

void printHotKeys() {
	std::cout << "\nHot Keys..\n"
		<< "Orthogonal Projection:  O\n"
		<< "Perspective Projection: P\n"
		<< "Toggle Spinning:        S\n"
		<< "Toggle Dolly Zoom:      D\n"
		<< "Show/hide Checkerboard: C\n"
		<< "Exit:                   Esc\n\n";
}

void clearModelViewMatrix()
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			g_modelViewMatrix[4 * i + j] = 0.0f;
		}
	}
}

void updateModelViewMatrix()
{	
	clearModelViewMatrix();
	if (teapotSpin) {
		//toggle spinning does not seem to work...It spins though.
		g_modelViewMatrix[0] = std::cos(.5f * (getTime()));
		g_modelViewMatrix[2] = std::sin(.5f * (getTime()));
		g_modelViewMatrix[8] = -std::sin(.5f * (getTime()));
		g_modelViewMatrix[5] = 1.0f;
		g_modelViewMatrix[10] = std::cos(.5f * (getTime()));

		g_modelViewMatrix[14] = -distance;
		g_modelViewMatrix[15] = 1.0f;
	}
	else {
		g_modelViewMatrix[0] = 1.0f;
		g_modelViewMatrix[5] = 1.0f;
		g_modelViewMatrix[10] = 1.0f;

		g_modelViewMatrix[14] = -distance;
		g_modelViewMatrix[15] = 1.0f;
	}
}

void setModelViewMatrix()
{
	glMatrixMode(GL_MODELVIEW);
	updateModelViewMatrix();
	glLoadMatrixf(g_modelViewMatrix);
}

void drawTeapot() {
	glBegin(GL_TRIANGLES);
	for (size_t f = 0; f < g_meshIndices.size(); ++f)
	{
		const float scale = 0.1f;
		const unsigned int idx = g_meshIndices[f];
		const float x = scale * g_meshVertices[3 * idx + 0];
		const float y = scale * g_meshVertices[3 * idx + 1];
		const float z = scale * g_meshVertices[3 * idx + 2];

		const float nx = g_meshNormals[3 * idx + 0];
		const float ny = g_meshNormals[3 * idx + 1];
		const float nz = g_meshNormals[3 * idx + 2];

		glNormal3f(nx, ny, nz);
		glVertex3f(x, y, z);
	}
	glEnd();
}
void renderTeapot() {
	drawTeapot();
}

void drawCheckerBoard() {
	int checkerCount = g_windowWidth;
	int checkerSize = (g_windowWidth) / checkerCount;

	glBegin(GL_QUADS);
	for (int i = 0; i < checkerCount; ++i) {
		for (int j = 0; j < checkerCount; ++j) {
			if ((i + j) % 2 == 0)
				glColor3f(0.0, 0.1, 1.0);
			else
				glColor3f(1.0, 0.0, 1.0);

			float x = i - checkerCount / 2; // to be between -1 and 1
			float z = j - checkerCount / 2;
			x *= checkerSize;
			z *= checkerSize;
			float y = -1.0f;
			glVertex3f(x, y, z);
			glVertex3f(x, y, z - checkerSize);
			glVertex3f(x + checkerSize, y, z - checkerSize);
			glVertex3f(x + checkerSize, y, z);
		}
	}
	glEnd();
}
void renderCheckerBoard() {

	/*
	/* If you want to keep checkerboard still while rotating the
	/* the teapot, you need to change the transformation for the
	/* checkerboard plane
	*/
	glMatrixMode(GL_MODELVIEW);
	clearModelViewMatrix();

	g_modelViewMatrix[0] = 1;
	g_modelViewMatrix[2] = 0;
	g_modelViewMatrix[5] = 1;
	g_modelViewMatrix[8] = 0;
	g_modelViewMatrix[10] = 1;
	g_modelViewMatrix[14] = -distance;
	g_modelViewMatrix[15] = 1.0f;
	
	glLoadMatrixf(g_modelViewMatrix);

	// Disable shading for the checkerboard
	glDisable(GL_LIGHTING);
	drawCheckerBoard();
	glEnable(GL_LIGHTING);
}

void render()
{
	togglePerspective();
	setModelViewMatrix();
	renderTeapot();
	if (showCheckerboard)
		renderCheckerBoard();
}

void renderLoop()
{
	while (!glfwWindowShouldClose(g_window))
	{
		// clear buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render();

		// Swap front and back buffers
		glfwSwapBuffers(g_window);

		// Poll for and process events
		glfwPollEvents();
	}
}

int main()
{
	initWindow();
	initGL();
	loadObj("data/teapot.obj");
	printHotKeys();
	renderLoop();
}
