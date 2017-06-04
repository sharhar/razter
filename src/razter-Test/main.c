#include <razter/razter.h>
#include <stdio.h>
#include <GLFW/glfw3native.h>

int main() {
	glfwInit();

	RZPlatform platform = RZ_PLATFORM_VULKAN;

	RZRenderContext* ctx = rzCreateRenderContext(platform);

	GLFWwindow* window = rzCreateWindow(ctx, 800, 600, "Razter Test");

	rzSetClearColor(ctx, 0.0f, 1.0f, 1.0f, 1.0f);

	float verts[] = {
		-0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f, -0.5f, 1.0f, 1.0f,

		-0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f,  0.5f, 0.0f, 0.0f
	};

	size_t offsets[] = { 0, sizeof(float) * 2};
	size_t stride = sizeof(float) * 4;
	uint32_t sizes[] = { 2, 2 };

	RZVertexAttributeDescription vertexAttribDesc;
	vertexAttribDesc.count = 2;
	vertexAttribDesc.stride = stride;
	vertexAttribDesc.offsets = offsets;
	vertexAttribDesc.sizes = sizes;

	RZBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.usage = RZ_BUFFER_USAGE_VERTEX;
	bufferCreateInfo.type = RZ_BUFFER_TYPE_STATIC;
	bufferCreateInfo.vertexAttribDesc = &vertexAttribDesc;

	RZBuffer* buffer = rzAllocateBuffer(ctx, &bufferCreateInfo, verts, sizeof(float) * 6 * 4);

	RZUniformDescriptor uniformDescriptors[3];
	uniformDescriptors[0].index = 0;
	uniformDescriptors[0].name = "view";
	uniformDescriptors[0].stage = RZ_UNIFORM_STAGE_VERTEX;
	uniformDescriptors[0].type = RZ_UNIFORM_TYPE_MATRIX_4;
	uniformDescriptors[0].bufferSize = sizeof(float) * 16;

	uniformDescriptors[1].index = 1;
	uniformDescriptors[1].name = "proj";
	uniformDescriptors[1].stage = RZ_UNIFORM_STAGE_VERTEX;
	uniformDescriptors[1].type = RZ_UNIFORM_TYPE_MATRIX_4;
	uniformDescriptors[1].bufferSize = sizeof(float) * 16;

	uniformDescriptors[2].index = 2;
	uniformDescriptors[2].name = "tex";
	uniformDescriptors[2].stage = RZ_UNIFORM_STAGE_FRAGMENT;
	uniformDescriptors[2].type = RZ_UNIFORM_TYPE_SAMPLED_IMAGE;
	uniformDescriptors[2].bufferSize = 0;

	RZShaderCreateInfo shaderCreateInfo;
	shaderCreateInfo.isPath = RZ_TRUE;
	shaderCreateInfo.vertexAttribDesc = &vertexAttribDesc;
	shaderCreateInfo.vertSize = 0;
	shaderCreateInfo.fragSize = 0;
	shaderCreateInfo.descriptors = uniformDescriptors;
	shaderCreateInfo.descriptorCount = 3;

	if (platform == RZ_PLATFORM_VULKAN) {
		shaderCreateInfo.vertData = "res/shader-vert.spv";
		shaderCreateInfo.fragData = "res/shader-frag.spv";
	}
#ifdef __APPLE__
	else if (platform == RZ_PLATFORM_METAL) {
		shaderCreateInfo.vertData = "vertex_function";
		shaderCreateInfo.fragData = "fragment_function";
	}
#endif
	

	RZShader* shader = rzCreateShader(ctx, &shaderCreateInfo);
	RZUniform* uniform = rzCreateUniform(ctx, shader);

	float view[] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	float proj[] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	rzUniformData(ctx, uniform, 0, view);
	rzUniformData(ctx, uniform, 1, proj);

	float imageData[] = {
		1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,
		1.0f, 0.3f, 0.3f,    0.3f, 1.0f, 0.3f,   0.3f, 0.3f, 1.0f,    0.3f, 0.3f, 0.3f,
		1.0f, 0.5f, 0.5f,    0.5f, 1.0f, 0.5f,   0.5f, 0.5f, 1.0f,    0.5f, 0.5f, 0.5f,
		1.0f, 0.8f, 0.8f,    0.8f, 1.0f, 0.8f,   0.8f, 0.8f, 1.0f,    0.8f, 0.8f, 0.8f
	};

	RZTextureCreateInfo textureCreateInfo;
	textureCreateInfo.width = 4;
	textureCreateInfo.height = 4;
	textureCreateInfo.data = imageData;
	textureCreateInfo.colorSize = RZ_COLOR_SIZE_FLOAT_32;
	textureCreateInfo.colorFormat = RZ_COLOR_FORMAT_RGB;

	RZTexture* texture = rzCreateTexture(ctx, &textureCreateInfo);

	rzUniformData(ctx, uniform, 2, texture);

	double ct = glfwGetTime();
	double dt = ct;

	float accDT = 0;
	uint32_t frames = 0;
	uint32_t fps = 0;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		dt = glfwGetTime() - ct;
		ct = glfwGetTime();

		accDT += dt;
		frames++;

		if (accDT > 1) {
			fps = frames;
			printf("FPS = %d\n", fps);
			frames = 0;
			accDT = 0;
		}

		view[12] = glfwGetTime()/10.0f;
		rzUniformData(ctx, uniform, 0, view);

		rzClear(ctx);
		
		rzBindBuffer(ctx, buffer);
		rzBindShader(ctx, shader);

		rzBindUniform(ctx, shader, uniform);

		rzDraw(ctx, 0, 6);

		rzSwap(ctx);
	}

	glfwTerminate();
	return 0;
}
