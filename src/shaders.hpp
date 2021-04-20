#include <glad/gl.h>
#include <iostream>

namespace shaders {
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


inline GLuint getShader(GLuint type, const char* text) {
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

struct circleShader {
	GLuint program, vertShader, fragShader, geomShader;
	GLuint positionLocation, radiusLocation, massLocation;
	GLuint transformLocation;
};

inline circleShader compileCircleShader() {
	circleShader out;

	out.vertShader = getShader(GL_VERTEX_SHADER, vertexShaderText);
	out.fragShader = getShader(GL_FRAGMENT_SHADER, fragShaderText);
	out.geomShader = getShader(GL_GEOMETRY_SHADER, geomShaderText);

	out.program = glCreateProgram();
	glAttachShader(out.program, out.vertShader);
	glAttachShader(out.program, out.fragShader);
	glAttachShader(out.program, out.geomShader);
	glLinkProgram(out.program);
	char* log = new char[1000];
	int len;

	glUseProgram(out.program);
	glGetProgramInfoLog(out.program, 1000, &len, log);
	std::cout << log << std::endl;
	
	delete[] log;

	out.positionLocation = glGetAttribLocation(out.program, "position");
	out.radiusLocation = glGetAttribLocation(out.program, "radius_in");
	out.massLocation = glGetAttribLocation(out.program, "mass_in");
	out.transformLocation = glGetUniformLocation(out.program, "transform");
	return out;
}

}
