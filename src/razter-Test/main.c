#include <razter/razter.h>
#include <stdio.h>

int main() {
	glfwInit();

	RZPlatform platform = RZ_PLATFORM_VULKAN;
	
#ifdef __APPLE__
	platform = RZ_PLATFORM_METAL;
#endif

	RZRenderContext* ctx = rzCreateRenderContext(platform);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Razter Test", NULL, NULL);
	
	RZCommandQueue** queues;
	RZSwapChain* swapChain;

	rzInitContext(ctx, window, &swapChain, RZ_TRUE, 1, &queues);
	RZCommandQueue* queue = queues[0];

	rzSetClearColor(ctx, swapChain, 0.0f, 1.0f, 1.0f, 1.0f);

	RZFrameBuffer* backBuffer = rzGetBackBuffer(ctx, swapChain);

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

	RZBuffer* buffer = rzAllocateBuffer(ctx, queue, &bufferCreateInfo, verts, sizeof(float) * 6 * 4);

	RZUniformDescriptor uniformDescriptors[2];
	uniformDescriptors[0].index = 1;
	uniformDescriptors[0].name = "view";
	uniformDescriptors[0].stage = RZ_UNIFORM_STAGE_VERTEX;
	uniformDescriptors[0].type = RZ_UNIFORM_TYPE_BUFFER;
	uniformDescriptors[0].bufferSize = sizeof(float) * 32;

	uniformDescriptors[1].index = 0;
	uniformDescriptors[1].name = "tex";
	uniformDescriptors[1].stage = RZ_UNIFORM_STAGE_FRAGMENT;
	uniformDescriptors[1].type = RZ_UNIFORM_TYPE_SAMPLED_IMAGE;
	uniformDescriptors[1].bufferSize = 0;

	RZShaderCreateInfo shaderCreateInfo;
	shaderCreateInfo.isPath = RZ_TRUE;
	shaderCreateInfo.vertexAttribDesc = &vertexAttribDesc;
	shaderCreateInfo.vertSize = 0;
	shaderCreateInfo.fragSize = 0;
	shaderCreateInfo.descriptors = uniformDescriptors;
	shaderCreateInfo.descriptorCount = 2;
	shaderCreateInfo.frameBuffer = backBuffer;
	
	shaderCreateInfo.vertData = "res/shader-vert.spv";
	shaderCreateInfo.fragData = "res/shader-frag.spv";

#ifdef __APPLE__
	shaderCreateInfo.vertFunction = "vertex_function";
	shaderCreateInfo.fragFunction = "fragment_function";
	
	shaderCreateInfo.vertData = "res/shader.metal";
	shaderCreateInfo.fragData = "res/shader.metal";
	
#endif
	

	RZShader* shader = rzCreateShader(ctx, &shaderCreateInfo);
	RZUniform* uniform = rzCreateUniform(ctx, shader);

	float view[] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
		
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	rzUniformData(ctx, uniform, 0, view);

	float imageData[] = {
		1.0f, 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.3f, 0.3f, 1.0f,    0.3f, 1.0f, 0.3f, 1.0f,   0.3f, 0.3f, 1.0f, 1.0f,    0.3f, 0.3f, 0.3f, 1.0f,
		1.0f, 0.5f, 0.5f, 1.0f,    0.5f, 1.0f, 0.5f, 1.0f,   0.5f, 0.5f, 1.0f, 1.0f,    0.5f, 0.5f, 0.5f, 1.0f,
		1.0f, 0.8f, 0.8f, 1.0f,    0.8f, 1.0f, 0.8f, 1.0f,   0.8f, 0.8f, 1.0f, 1.0f,    0.8f, 0.8f, 0.8f, 1.0f
	};

	RZTextureCreateInfo textureCreateInfo;
	textureCreateInfo.width = 4;
	textureCreateInfo.height = 4;
	textureCreateInfo.data = imageData;
	textureCreateInfo.componentsPerPixel = 4;
	textureCreateInfo.bytesPerComponent = sizeof(float);
	textureCreateInfo.componentType = RZ_COMPONENT_TYPE_FLOAT_32;

	RZTexture* texture = rzCreateTexture(ctx, queue, &textureCreateInfo);

	rzUniformData(ctx, uniform, 1, texture);

	RZCommandBuffer* cmdBuffer = rzCreateCommandBuffer(ctx, queue);

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
		
		rzStartCommandBuffer(ctx, queue, cmdBuffer);
		rzStartRender(ctx, backBuffer, cmdBuffer);
		
		rzBindBuffer(ctx, cmdBuffer, buffer);
		rzBindShader(ctx, cmdBuffer, shader);
		rzBindUniform(ctx, cmdBuffer, shader, uniform);
		
		rzDraw(ctx, cmdBuffer, 0, 6);
		
		rzEndRender(ctx, backBuffer, cmdBuffer);
		rzEndCommandBuffer(ctx, cmdBuffer);
		
		rzExecuteCommandBuffer(ctx, queue, cmdBuffer);

		rzPresent(ctx, swapChain);
	}

	glfwTerminate();
	return 0;
}
