#ifndef SHADERS_H
#define SHADERS_H

const char* vertShaderSource = R"(
#version 460 core
layout(location = 0) in vec3 aPos;
void main() {
	gl_Position = vec4(aPos, 1.0f);
}
)";

const char* fragShaderSource = R"(
#version 460 core
out vec3 fragColor;
void main() {
	fragColor = vec4(1.0f,0.0f,0.0f,1.0f);
}
)";

#endif