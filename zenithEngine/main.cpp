#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include<stdlib.h>
#define WIDTH 1280
#define HEIGHT 720
#define TITLE "i hate opengl"
int main() {
	glfwInit();
	glfwWindowHint(0x00022002, 4);
	glfwWindowHint(0x00022003, 6);
	glfwWindowHint(0x00022008, 0x00032001);
}