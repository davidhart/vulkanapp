#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// Uniform
layout (std140, binding = 0) uniform buf
{
	mat4 MVP;
} ubuf;

// In
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 attr;

// Out
layout (location = 0) out vec4 color;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
   color = vec4(attr.xyz, 1.0);
   gl_Position = ubuf.MVP * vec4(pos.xyz, 1.0);
}
