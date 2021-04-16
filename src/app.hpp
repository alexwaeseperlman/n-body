#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <quadtree/quadtree.hpp>
#include <iostream>

GLFWwindow *createWindow(int width, int height, const char *title);

using KeyCallback = void(GLFWwindow *window, int key, int scancode, int action, int mods);
using ClickCallback = void(GLFWwindow *window, int button, int action, int mods);
using CursorPosCallback = void(GLFWwindow* window, double xpos, double ypos);

class App {
	public:
		void start();

		App(GLFWwindow* setup(), bool draw(), KeyCallback keyCb, ClickCallback clickCb, CursorPosCallback cursorPosCb);
		~App();

		static void errorCallback(int error, const char* description) {
			std::cerr << "Error " << error << ": " << description << std::endl;
		}


		GLFWwindow* (*setup)();
		bool (*draw)();
		KeyCallback *keyCb;
		ClickCallback *clickCb;
		CursorPosCallback *cursorPosCb;

};
