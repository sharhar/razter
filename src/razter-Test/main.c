#include <razter/razter.h>
#include <stdio.h>

int main() {
	glfwInit();

	RZPlatform platform = RZ_PLATFORM_METAL;

	RZRenderContext* ctx = rzCreateRenderContext(platform);

	GLFWwindow* window = rzCreateWindow(ctx, 800, 600, "Razter Test");

	rzSetClearColor(ctx, 0.0f, 1.0f, 1.0f, 1.0f);

	float verts[] = {
		-0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f
	};

	size_t offsets[] = { 0, sizeof(float) * 2, sizeof(float) * 5 };
	size_t stride = sizeof(float) * 7;
	uint32_t sizes[] = { 2, 3, 2 };

	RZVertexAttributeDescription vertexAttribDesc;
	vertexAttribDesc.count = 3;
	vertexAttribDesc.stride = stride;
	vertexAttribDesc.offsets = offsets;
	vertexAttribDesc.sizes = sizes;

	RZBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.usage = RZ_BUFFER_USAGE_VERTEX;
	bufferCreateInfo.type = RZ_BUFFER_TYPE_STATIC;
	bufferCreateInfo.vertexAttribDesc = &vertexAttribDesc;

	RZBuffer* buffer = rzAllocateBuffer(ctx, &bufferCreateInfo, verts, sizeof(float) * 3 * 7);

	RZShaderCreateInfo shaderCreateInfo;
	shaderCreateInfo.isPath = RZ_TRUE;
	shaderCreateInfo.vertexAttribDesc = &vertexAttribDesc;
	shaderCreateInfo.vertSize = 0;
	shaderCreateInfo.fragSize = 0;

	if (platform == RZ_PLATFORM_VULKAN) {
		shaderCreateInfo.vertData = "res/shader-vert.spv";
		shaderCreateInfo.fragData = "res/shader-frag.spv";
	} else if (platform == RZ_PLATFORM_OPENGL) {
		shaderCreateInfo.vertData = "res/shader.vert";
		shaderCreateInfo.fragData = "res/shader.frag";
	}
	

	RZShader* shader = rzCreateShader(ctx, &shaderCreateInfo);

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

		rzClear(ctx);
		
		

		rzBindBuffer(ctx, buffer);
		rzBindShader(ctx, shader);
		rzDraw(ctx, 0, 3);

		rzSwap(ctx);
	}

	glfwTerminate();
	return 0;
}
