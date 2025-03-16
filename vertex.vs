#version 460 core
//#extension GL_NV_gpu_shader5 : enable

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;


uniform mat4 view;
uniform mat4 projection;

out vec4 v_Color;
out vec3 vPos;


void main()
{
    v_Color = aColor;
    vPos = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);

} 