#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "simulation.hpp"
#include "shaders.hpp"
#include "app.hpp"

GLFWwindow* window;
int width, height;

bool running = true;

void errorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::cerr << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "DEBUG ERROR" : "") << " type = " << type << " severity = " << severity << " message = " << message << std::endl;
}

simulation::Simulation sim;
GLuint pointsBuffer, pointArray;
shaders::circleShader circleDrawer;

void bindCircleDrawer(shaders::circleShader, GLuint, GLuint);

static GLFWwindow* setup() {
	for (int i = -5; i < 5; i++) {
		simulation::Body body = { .position = {.5f, (float)i / 5.f}, .radius = 0.05f, .mass = 0.05f, .velocity = {0.01f, 0.f} };
		sim.addBody(body);
	}

	window = createWindow(480, 360, "NBody");
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(errorCallback, 0);

	glGenVertexArrays(1, &pointArray);
	glBindVertexArray(pointArray);

	glGenBuffers(1, &pointsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, pointsBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(simulation::Body) * sim.getData().size(), &sim.getData()[0], GL_DYNAMIC_DRAW);

	glBindVertexBuffer(0, pointsBuffer, 0, sizeof(simulation::Body));

	circleDrawer = shaders::compileCircleShader();
	bindCircleDrawer(circleDrawer, pointArray, pointsBuffer);

	return window;
}

static bool draw() {
	//std::cout << "Error: " << glGetError() << std::endl;
	sim.step(0.1f);
	glBindBuffer(GL_ARRAY_BUFFER, pointsBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(simulation::Body) * sim.getData().size(), &sim.getData()[0]);

	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindBuffer(GL_ARRAY_BUFFER, pointsBuffer);
	glm::mat4 camera({
		{(float)std::min(width, height) / (float)width, 0.f, 0.f, 0.f},
		{0.f, (float)std::min(width, height) / (float)height, 0.f, 0.f},
		{ 0.f, 0.f, 1.f, 0.f },
		{ 0.f, 0.f, 0.f, 1.f }
	});

	glUniformMatrix4fv(circleDrawer.transformLocation, 1, GL_FALSE, glm::value_ptr((camera)));
	glUseProgram(circleDrawer.program);
	glDrawArrays(GL_POINTS, 0, sim.getData().size());

	return running;
}

static void keyCb(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE) running = false;
}

static void clickCb(GLFWwindow *window, int button, int action, int mods) {

}

static void cursorPosCb(GLFWwindow *window, double xpos, double ypos) {
	
}

void bindCircleDrawer(shaders::circleShader shader, GLuint arr, GLuint buffer) {
	glBindVertexArray(arr);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	
	glEnableVertexAttribArray(circleDrawer.positionLocation);
	std::cout << "Error: " << glGetError() << std::endl;
	glVertexAttribPointer(circleDrawer.positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(simulation::Body), (void*)0);
	std::cout << "Error: " << glGetError() << std::endl;

	glEnableVertexAttribArray(circleDrawer.radiusLocation);
	std::cout << "Error: " << glGetError() << std::endl;
	glVertexAttribPointer(circleDrawer.radiusLocation, 1, GL_FLOAT, GL_FALSE, sizeof(simulation::Body), (void*)(sizeof(float) * 2));
	std::cout << "Error: " << glGetError() << std::endl;

	glEnableVertexAttribArray(circleDrawer.massLocation);
	std::cout << "Error: " << glGetError() << std::endl;
	glVertexAttribPointer(circleDrawer.massLocation, 1, GL_FLOAT, GL_FALSE, sizeof(simulation::Body), (void*)(sizeof(float) * 3));
	std::cout << "Error: " << glGetError() << std::endl;

}

int main() {
	App app(&setup, &draw, &keyCb, &clickCb, &cursorPosCb);
	app.start();
}
