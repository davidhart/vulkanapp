#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 attr;
layout (location = 0) out vec4 color;

void main() {
   color = attr;
   gl_Position = pos;
}
