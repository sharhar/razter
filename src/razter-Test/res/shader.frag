#version 420 core
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in struct frag_in {
    vec2 uv;
	vec3 color;
} IN;

layout ( binding = 1 ) uniform sampler2D tex;

layout( std140, binding = 2 ) uniform RenderSettings {
	vec4 color;
	float width;
	float edge;
} RS;

layout (location = 0) out vec4 outColor;

void main(void) {
	outColor = vec4(IN.uv.xy, 0.0, 1.0);
}