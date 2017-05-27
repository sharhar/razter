#version 420 core
#extension GL_ARB_separate_shader_objects : enable

layout ( location = 0 ) in vec2 position;
layout ( location = 1 ) in vec3 color;

layout( location = 0 ) out vec3 col;

void main(void) {
	gl_Position = vec4(position.xy, 0.0, 1.0);
	col = color;
}