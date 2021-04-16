#include <glad/gl.h>
#include "app.hpp"

GLFWwindow* window;
int width, height;

bool running = true;

static const struct {
	float x, y;
	float r;
} points[3] = {
	{0.f, 0.f, 0.2f},
	{0.5f, -0.1f, 0.2f},
	{-0.8f, 0.8f, 0.4f}
};

static GLFWwindow* setup() {
	window = createWindow(480, 360, "NBody");
	return window;
}

static bool draw() {
	glfwGetFramebufferSize(window, &width, &height);
	glClearColor(0.5f, 0.f, 0.f, 0.f);
	glad_glViewport(0, 0, 100, 100);
	glClear(GL_COLOR_BUFFER_BIT);
	return running;
}

static void keyCb(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE) running = false;
}

static void clickCb(GLFWwindow *window, int button, int action, int mods) {

}

static void cursorPosCb(GLFWwindow *window, double xpos, double ypos) {
	
}

int main() {
	App app(&setup, &draw, &keyCb, &clickCb, &cursorPosCb);
	app.start();
}
