#include <razter/razter.h>
#include <stdio.h>

int main() {
	glfwInit();

	RZAPI api = RZ_API_VULKAN;
	
#ifdef __APPLE__
	api = RZ_API_METAL;
#endif

	RZContext* ctx = rzCreateContext(api);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Razter Test", NULL, NULL);
	
    RZDevice* device;
	RZCommandQueue** queues;
	RZSwapChain* swapChain;
    
	ctx->createDevice(&device, window, &swapChain, RZ_TRUE, 1, &queues);
	RZCommandQueue* queue = queues[0];

	ctx->setClearColor(swapChain, 0.0f, 1.0f, 1.0f, 1.0f);

	RZFrameBuffer* backBuffer = ctx->getBackBuffer(swapChain);

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

	RZBuffer* buffer = ctx->allocBuffer(device, queue, &bufferCreateInfo, verts, sizeof(float) * 6 * 4);

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
	

	RZShader* shader = ctx->createShader(device, &shaderCreateInfo);
	RZUniform* uniform = ctx->createUniform(device, shader);

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

	ctx->uniformData(device, uniform, 0, view);

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

	RZTexture* texture = ctx->createTexture(device, queue, &textureCreateInfo);

	ctx->uniformData(device, uniform, 1, texture);

	RZCommandBuffer* cmdBuffer = ctx->createCommandBuffer(device, queue);

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
		ctx->uniformData(device, uniform, 0, view);
		
		ctx->startCommandBuffer(device, queue, cmdBuffer);
		ctx->startRender(device, backBuffer, cmdBuffer);
		
		ctx->bindBuffer(device, cmdBuffer, buffer);
		ctx->bindShader(device, cmdBuffer, shader);
		ctx->bindUniform(device, cmdBuffer, shader, uniform);
		
		ctx->draw(device, cmdBuffer, 0, 6);
		
		ctx->endRender(device, backBuffer, cmdBuffer);
		ctx->endCommandBuffer(device, cmdBuffer);
		
		ctx->executeCommandBuffer(device, queue, cmdBuffer);

		ctx->present(device, swapChain);
	}

	glfwTerminate();
	return 0;
}
