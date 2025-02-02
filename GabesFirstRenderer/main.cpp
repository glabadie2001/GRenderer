#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include <string>
#include <vector>

#include "ShaderManager.h"
#include "MeshManager.h"

using namespace std;

MeshManager meshManager;
ShaderManager shaderManager;
unsigned int shaderProgram;

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

void processInput(GLFWwindow* window);
void initGLAD(), initGeometry(), renderLoop();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLFWwindow* create_window(int width, int height);
unsigned int setupShaderProgram();

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void initGLAD() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		exit(1);
	}
}

/*
 * Called every time the window is resized
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

GLFWwindow* create_window(int width, int height) {
	GLFWwindow* window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		exit(1);
	}
	glfwMakeContextCurrent(window);

	return window;
}

unsigned int setupShaderProgram() {
	shaderManager.CompileAndAdd(vertexShaderSource, GL_VERTEX_SHADER);
	shaderManager.CompileAndAdd(fragShaderSource, GL_FRAGMENT_SHADER);

	return shaderManager.BuildAndLink();
}

void initGeometry() {
	float triVerts[] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.0f,  0.5f, 0.0f
	};

	float rectVerts[] = {
			0.5f,  0.5f, 0.0f,  // top right
			0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
	};
	unsigned int rectIndices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	//meshManager.Add(new Mesh(triVerts, sizeof(triVerts)));
	meshManager.Add(new Mesh(rectVerts, sizeof(rectVerts), rectIndices, sizeof(rectIndices)));
}

void renderLoop() {
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);

	meshManager.RenderAll();
}

int main() {
	/*
	 * Initialization
	 */
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = create_window(800, 600);

	initGLAD();

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	shaderProgram = setupShaderProgram();
	initGeometry();
	/*
	 * Render Loop!
	 */
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		renderLoop();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	/* Clean up all resources associated with our window */
	//TODO: Make this an actual function for when I have my own resources to clean up
	glfwTerminate();
	return 0;
}