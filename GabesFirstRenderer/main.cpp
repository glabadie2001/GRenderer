#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include <string>
#include <vector>

#include "Scene.h"
#include "Renderer.h"
#include "Shader.h"

using namespace std;

Renderer renderer;
Scene* scene;
Shader* densityShader;
Shader* particleShader;

void processInput(GLFWwindow* window);
void initGLAD(), initGeometry(), setupDefaultShader();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLFWwindow* create_window(int width, int height);
Scene *testScene(), *particleScene();

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

void setupDefaultShader() {
	//densityShader = new Shader("ParticleVertShader.glsl", "DensityFragment.glsl");
	particleShader = new Shader("ParticleVertShader.glsl", "FragmentShader.glsl");
}

void initGeometry() {
	scene = particleScene();
}

Scene* testScene() {
	Scene* testScene = new Scene();

	float triVerts[] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.0f,  0.5f, 0.0f
	};

	float rectVerts[] = {
			0.0f,  0.5f, 0.0f,  // top right
			0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
	};
	unsigned int rectIndices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	//testScene->Add(new Mesh(triVerts, sizeof(triVerts), defaultShader));
	testScene->add(new Mesh(rectVerts, sizeof(rectVerts), rectIndices, sizeof(rectIndices), densityShader));

	return testScene;
}

Scene* particleScene() {
	glEnable(GL_PROGRAM_POINT_SIZE);
	Scene* partScene = new Scene();
	int i;
	int pCount = 500;

	partScene->add(new ParticleSystem(pCount, particleShader, 800, 600));

	//float rectVerts[] = {
	//	1.0f,  1.0f, 0.0f,  // top right
	//	1.0f, -1.0f, 0.0f,  // bottom right
	//-1.0f, -1.0f, 0.0f,  // bottom left
	//-1.0f,  1.0f, 0.0f   // top left 
	//};
	//unsigned int rectIndices[] = {  // note that we start from 0!
	//	0, 1, 3,   // first triangle
	//	1, 2, 3    // second triangle
	//};

	//partScene->add(new Mesh(rectVerts, sizeof(rectVerts), rectIndices, sizeof(rectIndices), particleShader));

	return partScene;
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

	/*
	 * Rendering prep
	 */
	setupDefaultShader();

	initGeometry();

	/*
	 * Render Loop!
	 */
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		scene->update();
		renderer.renderLoop(*scene);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	/* Clean up all resources associated with our window */
	//TODO: Make this an actual function for when I have my own resources to clean up
	glfwTerminate();

	delete scene;
	delete densityShader;
	//delete window;

	return 0;
}