#version 420 core
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in vec3 col;

layout (location = 0) out vec4 outColor;

void main(void) {
	outColor = vec4(col.xyz, 1.0);
}