#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include <string>
#include <vector>

#include "Scene.h"
#include "Renderer.h"
#include "Shader.h"

using namespace std;

GLFWwindow* window;
Renderer renderer;
Scene* scene;
Shader* densityShader;
Shader* particleShader;

float prevTime, currTime, deltaTime;

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
	renderer.updateScreenSize(*scene, width, height);
	scene->update(deltaTime);
	renderer.renderLoop(*scene);

	glfwSwapBuffers(window);
}

void window_pos_callback(GLFWwindow* window, int x, int y)
{
	renderer.updateWindowPosition(*scene, x, y);
	scene->update(deltaTime);
	renderer.renderLoop(*scene);

	glfwSwapBuffers(window);
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
	particleShader = new Shader("ParticleVertShader.vert", "FragmentShader.frag");
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
	int pCount = 10000;

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	int x, y;
	glfwGetWindowPos(window, &x, &y);

	partScene->add(new ParticleSystem(pCount, particleShader, width, height, x, y));

	return partScene;
}

int main() {
	float lastTime = (float)glfwGetTime();
	int nbFrames = 0;
	currTime = 0;

	/*
	 * Initialization
	 */
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = create_window(800, 600);

	initGLAD();

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetWindowPosCallback(window, window_pos_callback);

	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "OpenGL Driver Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;

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
		std::cout << "------------------- START FRAME -------------------" << std::endl;
		// Measure speed
		prevTime = currTime;
		currTime = (float)glfwGetTime();
		deltaTime = currTime - prevTime;
		nbFrames++;
		if (currTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
			// printf and reset timer
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		processInput(window);

		scene->update(deltaTime);
		renderer.renderLoop(*scene);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	/* Clean up all resources associated with our window */
	//TODO: Make this an actual function for when I have my own resources to clean up
	glfwTerminate();

	delete scene;
	delete densityShader;
	//delete particleShader;

	return 0;
}