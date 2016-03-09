#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 attr;
layout (location = 0) out vec4 color;

void main() {
   color = vec4(attr.xyz, 1.0);
   gl_Position = vec4(pos.xyz, 1.0);
}
