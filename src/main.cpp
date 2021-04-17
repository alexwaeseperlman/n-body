#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "app.hpp"

GLFWwindow* window;
int width, height;

bool running = true;

static const char* vertexShaderText = R"glsl(
#version 450
layout(location=0) in vec2 position;
layout(location=1) in float radius_in;
layout(location=2) in float mass_in;

out mat4 transform;
out float radius;
out float mass_out;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
	radius = radius_in;
	mass_out = mass_in;
};)glsl";

static const char* fragShaderText = R"glsl(
#version 450
out vec4 FragColor;
in float mass;
void main() {
	FragColor = vec4(mass, 1.0, 0.0, 1.0);
};)glsl";

static const char* geomShaderText = R"glsl(
#version 450
layout(points) in;
layout(triangle_strip, max_vertices=60) out;

uniform mat4 transform;

in float radius[];
in float mass_out[];

out float mass;

void drawCircle() {
	int sides = 20;
	vec4 transformed = transform * gl_in[0].gl_Position;
	for (int i = 0; i < sides + 1; i += 2) {
		float angle = radians((360 / sides) * i);
		float nangle = radians((360 / sides) * i + (360 / sides));

		gl_Position = transformed + transform * (vec4(radius[0] * cos(angle), radius[0] * sin(angle), 0.0, 1.0));
		mass = mass_out[0];
		EmitVertex();

		gl_Position = transformed + transform * (vec4(0.0, 0.0, 0.0, 1.0));
		mass = mass_out[0];
		EmitVertex();

		gl_Position = transformed + transform * (vec4(radius[0] * cos(nangle), radius[0] * sin(nangle), 0.0, 1.0));
		mass = mass_out[0];
		EmitVertex();
	}

	EndPrimitive();
}

void main() {
	drawCircle();
})glsl";

void errorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::cerr << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "DEBUG ERROR" : "") << " type = " << type << " severity = " << severity << " message = " << message << std::endl;
}

GLuint getShader(GLuint type, const char* text) {
	std::cout << "Compiling shader" << std::endl;
	GLuint outShader = glCreateShader(type);
	glShaderSource(outShader, 1, &text, NULL);
	glCompileShader(outShader);

	GLint success = 0;
	glGetShaderiv(outShader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		std::cerr << "Unable to compile shader" << std::endl;
		GLint logSize = 0;
		glGetShaderiv(outShader, GL_INFO_LOG_LENGTH, &logSize);
		char* shaderLog = new char[logSize];
		glGetShaderInfoLog(outShader, logSize, &logSize, shaderLog);
		std::cerr << shaderLog << std::endl;

		delete[] shaderLog;
	}
	return outShader;
}

static struct {
	float x, y;
	float r, mass;
} points[3] = {
	{0.f, 0.f, 0.08f, 1.f},
	{0.5f, -0.1f, 0.08f, 1.f},
	{-0.8f, 0.8f, 0.08f, 1.f}
};

static struct {
	GLuint program, vertShader, fragShader, geomShader;
	GLuint positionLocation, radiusLocation, massLocation;
	GLuint transformLocation;
} circleDrawer;

GLuint pointsBuffer, pointArray;

static GLFWwindow* setup() {
	window = createWindow(480, 360, "NBody");
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(errorCallback, 0);

	glGenVertexArrays(1, &pointArray);
	glBindVertexArray(pointArray);

	glGenBuffers(1, &pointsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, pointsBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_DYNAMIC_DRAW);

	glBindVertexBuffer(0, pointsBuffer, 0, sizeof(points[0]));

	circleDrawer.vertShader = getShader(GL_VERTEX_SHADER, vertexShaderText);
	circleDrawer.fragShader = getShader(GL_FRAGMENT_SHADER, fragShaderText);
	circleDrawer.geomShader = getShader(GL_GEOMETRY_SHADER, geomShaderText);

	circleDrawer.program = glCreateProgram();
	glAttachShader(circleDrawer.program, circleDrawer.vertShader);
	glAttachShader(circleDrawer.program, circleDrawer.fragShader);
	glAttachShader(circleDrawer.program, circleDrawer.geomShader);
	glLinkProgram(circleDrawer.program);
	char* log = new char[1000];
	int len;

	glGetProgramInfoLog(circleDrawer.program, 1000, &len, log);
	std::cout << log << std::endl;
	
	delete[] log;
	glUseProgram(circleDrawer.program);


	circleDrawer.positionLocation = glGetAttribLocation(circleDrawer.program, "position");
	circleDrawer.radiusLocation = glGetAttribLocation(circleDrawer.program, "radius_in");
	circleDrawer.massLocation = glGetAttribLocation(circleDrawer.program, "mass_in");
	circleDrawer.transformLocation = glGetUniformLocation(circleDrawer.program, "transform");

	glEnableVertexAttribArray(circleDrawer.positionLocation);
	std::cout << "Error: " << glGetError() << std::endl;
	glVertexAttribPointer(circleDrawer.positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(points[0]), (void*)0);
	std::cout << "Error: " << glGetError() << std::endl;

	glEnableVertexAttribArray(circleDrawer.radiusLocation);
	std::cout << "Error: " << glGetError() << std::endl;
	glVertexAttribPointer(circleDrawer.radiusLocation, 1, GL_FLOAT, GL_FALSE, sizeof(points[0]), (void*)(sizeof(float) * 2));
	std::cout << "Error: " << glGetError() << std::endl;

	glEnableVertexAttribArray(circleDrawer.massLocation);
	std::cout << "Error: " << glGetError() << std::endl;
	glVertexAttribPointer(circleDrawer.massLocation, 1, GL_FLOAT, GL_FALSE, sizeof(points[0]), (void*)(sizeof(float) * 3));
	std::cout << "Error: " << glGetError() << std::endl;

	return window;
}

static bool draw() {
	//std::cout << "Error: " << glGetError() << std::endl;

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
	glDrawArrays(GL_POINTS, 0, 3);

	points[0].x += 0.001f;
	points[1].y += 0.001f;

	glBindBuffer(GL_ARRAY_BUFFER, pointsBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points[0]) * 2, points);

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
