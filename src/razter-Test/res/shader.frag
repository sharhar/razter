#version 400 core
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in struct frag_in {
	vec2 uv;
	vec3 color;
} IN;

uniform sampler2D tex;

uniform vec4 color;
uniform float width;
uniform float edge;

layout (location = 0) out vec4 outColor;

void main(void) {
	outColor = vec4(IN.uv.xy, 0.0, 1.0);
}
