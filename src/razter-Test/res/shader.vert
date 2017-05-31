#version 400 core
#extension GL_ARB_separate_shader_objects : enable

layout ( location = 0 ) in vec2 position;
layout ( location = 1 ) in vec3 color;
layout ( location = 2 ) in vec2 uv;

uniform mat4 view;
uniform mat4 proj;

layout( location = 0 ) out struct vert_out {
	vec2 uv;
	vec3 color;
} OUT;

void main(void) {
	gl_Position = vec4(position.xy, 0.0, 1.0);
	OUT.color = color;
	OUT.uv = uv;
}
