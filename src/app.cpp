#include <glad/gl.h>
#include "app.hpp"

App::~App() {
	glfwTerminate();
}

	
App::App(GLFWwindow* setup(), bool draw(), KeyCallback keyCb, ClickCallback clickCb, CursorPosCallback cursorPosCb): setup(setup), draw(draw), keyCb(keyCb), clickCb(clickCb), cursorPosCb(cursorPosCb) {
	glfwSetErrorCallback(errorCallback);
	if (!glfwInit()) {
		errorCallback(1, "GLFW unable to initialize");
	}
}

GLFWwindow* createWindow(int width, int height, const char *title) {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);

	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);

	return window;
}

void App::start() {
	GLFWwindow* window = setup();
	glfwSetCursorPosCallback(window, cursorPosCb);
	glfwSetMouseButtonCallback(window, clickCb);
	glfwSetKeyCallback(window, keyCb);

	glfwSwapInterval(1);

	while (!glfwWindowShouldClose(window) && draw()) {
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	
	glfwDestroyWindow(window);
	glfwTerminate();
}
