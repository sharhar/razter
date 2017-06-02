#include <razter/razter.h>
#include <memory.h>
#include <VKL/VKL.h>

typedef struct VKCTX {
	VKLInstance* instance;
	VKLSurface* surface;
	VKLDevice* device;
	VKLDeviceGraphicsContext* devCon;
	VKLSwapChain* swapChain;
	VkCommandBuffer cmdBuffer;
} VKCTX;

typedef struct VKBuffer {
	VKLBuffer* buffer;
} VKBuffer;

typedef struct  VKShader{
	VKLShader* shader;
	VKLGraphicsPipeline* pipeline;
	RZUniformDescriptor* descriptors;
	uint32_t descriptorCount;
} VKShader;

typedef struct VKUniform {
	VKLUniformObject* uniform;
	VKLBuffer** buffers;
	uint32_t bufferCount;
} VKUniform;

void rzvkInit(RZRenderContext* ctx) {
	ctx->ctx = (VKCTX*)malloc(sizeof(VKCTX));
}

GLFWwindow* rzvkCreateWindow(RZRenderContext* ctx, int width, int height, const char* title) {
	VKCTX* vkCTX = (VKCTX*)ctx->ctx;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Razter Test", NULL, NULL);

	vklCreateInstance(&vkCTX->instance, NULL, VK_TRUE, NULL);
	vklCreateGLFWSurface(vkCTX->instance, &vkCTX->surface, window);
	VKLDeviceGraphicsContext** deviceContexts;
	vklCreateDevice(vkCTX->instance, &vkCTX->device, &vkCTX->surface, 1, &deviceContexts, 0, NULL);
	vkCTX->devCon = deviceContexts[0];
	vklCreateSwapChain(vkCTX->devCon, &vkCTX->swapChain, VK_TRUE);

	vklAllocateCommandBuffer(vkCTX->devCon, &vkCTX->cmdBuffer, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

	return window;
}

void rzvkClear(RZRenderContext* ctx) {
	VKCTX* vkCTX = (VKCTX*)ctx->ctx;

	vklClearScreen(vkCTX->swapChain);
	vklBeginRenderRecording(vkCTX->swapChain, vkCTX->cmdBuffer);

	VkViewport viewport = { 0, 0, vkCTX->swapChain->width, vkCTX->swapChain->height, 0, 1 };
	VkRect2D scissor = {
		{0, 0},
		{vkCTX->swapChain->width, vkCTX->swapChain->height} };
	
	vkCTX->device->pvkCmdSetViewport(vkCTX->cmdBuffer, 0, 1, &viewport);
	vkCTX->device->pvkCmdSetScissor(vkCTX->cmdBuffer, 0, 1, &scissor);
}

void rzvkSetClearColor(RZRenderContext* ctx, float r, float g, float b, float a) {
	VKCTX* vkCTX = (VKCTX*)ctx->ctx;
	vklSetClearColor(vkCTX->swapChain, r, g, b, a);
}

void rzvkSwap(RZRenderContext* ctx) {
	VKCTX* vkCTX = (VKCTX*)ctx->ctx;

	vklEndRenderRecording(vkCTX->swapChain, vkCTX->cmdBuffer);
	vklRenderRecording(vkCTX->swapChain, vkCTX->cmdBuffer);
	vklSwapBuffers(vkCTX->swapChain);
}

RZBuffer* rzvkAllocateBuffer(RZRenderContext* ctx, RZBufferCreateInfo* createInfo, void* data, size_t size) {
	VKCTX* vkCTX = (VKCTX*)ctx->ctx;

	VKBuffer* buffer = malloc(sizeof(VKBuffer));

	if (createInfo->type == RZ_BUFFER_TYPE_DYNAMIC) {
		vklCreateBuffer(vkCTX->device, &buffer->buffer, VK_FALSE, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		vklWriteToMemory(vkCTX->device, buffer->buffer->memory, data, size);
	} else if (createInfo->type == RZ_BUFFER_TYPE_STATIC) {
		vklCreateStagedBuffer(vkCTX->devCon, &buffer->buffer, data, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	}
	
	return buffer;
}

void rzvkUpdateBuffer(RZRenderContext* ctx, RZBuffer* buffer, void* data, size_t size) {

}

void rzvkBindBuffer(RZRenderContext* ctx, RZBuffer* buffer) {
	VKCTX* vkCTX = (VKCTX*)ctx->ctx;
	VKBuffer* vkBuff = (VKBuffer*)buffer;

	VkDeviceSize offsets = 0;
	vkCTX->device->pvkCmdBindVertexBuffers(vkCTX->cmdBuffer, 0, 1, &vkBuff->buffer->buffer, &offsets);
}

void rzvkFreeBuffer(RZRenderContext* ctx, RZBuffer* buffer) {

}

RZShader* rzvkCreateShader(RZRenderContext* ctx, RZShaderCreateInfo* createInfo) {
	VKCTX* vkCTX = (VKCTX*)ctx->ctx;

	VKShader* shader = malloc(sizeof(VKShader));
	
	shader->descriptorCount = createInfo->descriptorCount;
	shader->descriptors = malloc_c(sizeof(RZUniformDescriptor) * shader->descriptorCount);
	memcpy(shader->descriptors, createInfo->descriptors, sizeof(RZUniformDescriptor) * shader->descriptorCount);

	char* shaderPaths[2];
	shaderPaths[0] = createInfo->vertData;
	shaderPaths[1] = createInfo->fragData;

	VkShaderStageFlagBits stages[2];
	stages[0] = VK_SHADER_STAGE_VERTEX_BIT;
	stages[1] = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkFormat* formats = malloc(sizeof(VkFormat) * createInfo->vertexAttribDesc->count);
	
	for (uint32_t i = 0; i < createInfo->vertexAttribDesc->count;i++) {
		uint32_t size = createInfo->vertexAttribDesc->sizes[i];
		if (size == 1) {
			formats[i] = VK_FORMAT_R32_SFLOAT;
		} else if (size == 2) {
			formats[i] = VK_FORMAT_R32G32_SFLOAT;
		} else if (size == 3) {
			formats[i] = VK_FORMAT_R32G32B32_SFLOAT;
		} else if (size == 4) {
			formats[i] = VK_FORMAT_R32G32B32A32_SFLOAT;
		}
	}

	VkDescriptorSetLayoutBinding* bindings = NULL;
	uint32_t bindingCount = 0;

	if (createInfo->descriptors != NULL) {
		bindingCount = createInfo->descriptorCount;
		bindings = malloc_c(sizeof(VkDescriptorSetLayoutBinding) * bindingCount);

		for (uint32_t i = 0; i < bindingCount; i++) {
			bindings[i].binding = createInfo->descriptors[i].index;
			bindings[i].descriptorCount = 1;
			bindings[i].pImmutableSamplers = NULL;

			RZUniformStage stage = createInfo->descriptors[i].stage;
			if (stage == RZ_UNIFORM_STAGE_VERTEX) {
				bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			} else if (stage == RZ_UNIFORM_STAGE_FRAGMENT) {
				bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			}

			RZUniformType type = createInfo->descriptors[i].type;
			if (type == RZ_UNIFORM_TYPE_SAMPLED_IMAGE) {
				bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			} else {
				bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			}
		}
	}

	VKLShaderCreateInfo shaderCreateInfo;
	memset(&shaderCreateInfo, 0, sizeof(VKLShaderCreateInfo));
	shaderCreateInfo.shaderPaths = shaderPaths;
	shaderCreateInfo.shaderStages = stages;
	shaderCreateInfo.shaderCount = 2;
	shaderCreateInfo.bindings = bindings;
	shaderCreateInfo.bindingsCount = bindingCount;
	shaderCreateInfo.vertexInputAttributeStride = createInfo->vertexAttribDesc->stride;
	shaderCreateInfo.vertexInputAttributesCount = createInfo->vertexAttribDesc->count;
	shaderCreateInfo.vertexInputAttributeOffsets = createInfo->vertexAttribDesc->offsets;
	shaderCreateInfo.vertexInputAttributeFormats = formats;

	vklCreateShader(vkCTX->device, &shader->shader, &shaderCreateInfo);

	VKLPipelineCreateInfo pipelineCreateInfo;
	memset(&pipelineCreateInfo, 0, sizeof(VKLPipelineCreateInfo));
	pipelineCreateInfo.shader = shader->shader;
	pipelineCreateInfo.renderPass = vkCTX->swapChain->renderPass;
	pipelineCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineCreateInfo.cullMode = VK_CULL_MODE_NONE;
	pipelineCreateInfo.extent.width = vkCTX->swapChain->width;
	pipelineCreateInfo.extent.height = vkCTX->swapChain->height;

	vklCreateGraphicsPipeline(vkCTX->device, &shader->pipeline, &pipelineCreateInfo);

	return shader;
}

void rzvkBindShader(RZRenderContext* ctx, RZShader* shader) {
	VKCTX* vkCTX = (VKCTX*)ctx->ctx;
	VKShader* vkShader = (VKShader*)shader;
	vkCTX->device->pvkCmdBindPipeline(vkCTX->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkShader->pipeline->pipeline);
}

void rzvkDestroyShader(RZRenderContext* ctx, RZShader* shader) {

}

void rzvkDraw(RZRenderContext* ctx, uint32_t firstVertex, uint32_t vertexCount) {
	VKCTX* vkCTX = (VKCTX*)ctx->ctx;
	vkCTX->device->pvkCmdDraw(vkCTX->cmdBuffer, vertexCount, 1, firstVertex, 0);
}

RZUniform* rzvkCreateUniform(RZRenderContext* ctx, RZShader* shader) {
	VKCTX* vkCTX = (VKCTX*)ctx->ctx;
	VKShader* vkShader = (VKShader*)shader;
	VKUniform* uniform = malloc_c(sizeof(VKUniform));

	vklCreateUniformObject(vkCTX->device, &uniform->uniform, vkShader->shader);

	uniform->bufferCount = vkShader->descriptorCount;
	uniform->buffers = malloc_c(sizeof(VKLBuffer*) * uniform->bufferCount);

	for (uint32_t i = 0; i < uniform->bufferCount;i++) {
		if (vkShader->descriptors[i].type != RZ_UNIFORM_TYPE_SAMPLED_IMAGE) {
			vklCreateBuffer(vkCTX->device, &uniform->buffers[i], VK_FALSE, vkShader->descriptors[i].bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
			vklSetUniformBuffer(vkCTX->device, uniform->uniform, uniform->buffers[i], vkShader->descriptors[i].index);
		}
	}

	return uniform;
}

void rzvkBindUniform(RZRenderContext* ctx, RZShader* shader, RZUniform* uniform) {
	VKCTX* vkCTX = (VKCTX*)ctx->ctx;
	VKShader* vkShader = (VKShader*)shader;
	VKUniform* vkUniform = (VKUniform*)uniform;

	vkCTX->device->pvkCmdBindDescriptorSets(vkCTX->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		vkShader->pipeline->pipelineLayout, 0, 1, &vkUniform->uniform->descriptorSet, 0, NULL);
}

void rzvkUniformData(RZRenderContext* ctx, RZUniform* uniform, uint32_t index, void* data) {
	VKCTX* vkCTX = (VKCTX*)ctx->ctx;
	VKUniform* vkUniform = (VKUniform*)uniform;

	vklWriteToMemory(vkCTX->device, vkUniform->buffers[index]->memory, data, vkUniform->buffers[index]->size);
}

void rzvkDestroyUniform(RZRenderContext* ctx, RZUniform* uniform) {
	
}

void rzvkLoadPFN(RZRenderContext* ctx) {
	ctx->init = rzvkInit;
	ctx->createWindow = rzvkCreateWindow;
	ctx->clear = rzvkClear;
	ctx->setClearColor = rzvkSetClearColor;
	ctx->swap = rzvkSwap;

	ctx->allocBuffer = rzvkAllocateBuffer;
	ctx->updateBuffer = rzvkUpdateBuffer;
	ctx->bindBuffer = rzvkBindBuffer;
	ctx->freeBuffer = rzvkFreeBuffer;

	ctx->createShader = rzvkCreateShader;
	ctx->bindShader = rzvkBindShader;
	ctx->destroyShader = rzvkDestroyShader;

	ctx->draw = rzvkDraw;

	ctx->createUniform = rzvkCreateUniform;
	ctx->bindUniform = rzvkBindUniform;
	ctx->uniformData = rzvkUniformData;
	ctx->destroyUniform = rzvkDestroyUniform;
}
