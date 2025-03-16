#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
out vec4 Color;

void main() {
    gl_Position = vec4(aPos, 1.0);
    gl_PointSize = 5.0;
    Color = aColor;
}