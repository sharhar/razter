#version 420 core
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in struct frag_in {
    vec2 uv;
} IN;

layout ( binding = 2 ) uniform sampler2D tex;

layout (location = 0) out vec4 outColor;

void main(void) {
	outColor = texture(tex, IN.uv);
}
