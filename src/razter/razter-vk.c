#include <razter/razter.h>
#include <memory.h>
#include <VKL/VKL.h>

typedef struct VKCTX {
	VKLInstance* instance;
	VKLSurface* surface;
	VKLDevice* device;
} VKCTX;

typedef struct VKFrameBuffer {
	VKLFrameBuffer* frameBuffer;
} VKFrameBuffer;

typedef struct VKSwapChain {
	VKLSwapChain* swapChain;
	VKFrameBuffer* backBuffer;
} VKSwapChain;

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
	uint32_t count;
	RZUniformType* types;
	uint32_t* indexes;
} VKUniform;

typedef struct VKTexture {
	VKLTexture* texture;
} VKTexture;

typedef struct VKCommandQueue {
	VKLDeviceGraphicsContext* devCon;
	VkCommandBuffer setupCmdBuffer;
} VKCommandQueue;

typedef struct VKCommandBuffer {
	VkCommandBuffer cmdBuffer;
} VKCommandBuffer;

void rzvkInitContext(VKCTX** pCtx, GLFWwindow* window, VKSwapChain** pSwapChain, RZBool debug, uint32_t queueCount, VKCommandQueue*** pQueues) {
    
	VKCTX* vctx = (VKCTX*)malloc(sizeof(VKCTX));

	VkBool32 vkDebug = VK_FALSE;

	if (debug == RZ_TRUE) {
		vkDebug = VK_TRUE;
	}

	vklCreateInstance(&vctx->instance, NULL, vkDebug, NULL);
	vklCreateGLFWSurface(vctx->instance, &vctx->surface, window);
	VKLDeviceGraphicsContext** deviceContexts;
	vklCreateDevice(vctx->instance, &vctx->device, &vctx->surface, queueCount, &deviceContexts, 0, NULL);

	VKCommandQueue** queues = malloc_c(sizeof(VKCommandQueue*));
	for (uint32_t i = 0; i < queueCount;i++) {
		queues[i] = malloc_c(sizeof(VKCommandQueue));
		queues[i]->devCon = deviceContexts[i];
	}

	VKSwapChain* swapChain = malloc(sizeof(VKSwapChain));

	vklCreateSwapChain(queues[0]->devCon, &swapChain->swapChain, VK_FALSE);
	
	VKFrameBuffer* backBuffer = malloc(sizeof(VKFrameBuffer));
	
	vklGetBackBuffer(swapChain->swapChain, &backBuffer->frameBuffer);

	swapChain->backBuffer = backBuffer;

	vklAllocateCommandBuffer(queues[0]->devCon, &queues[0]->setupCmdBuffer, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

	*pQueues = queues;
	*pSwapChain = swapChain;
	*pCtx = vctx;
}

RZFrameBuffer* rzvkGetBackBuffer(VKSwapChain* swapChain) {
	return swapChain->backBuffer;
}

void rzvkSetClearColor(VKSwapChain* swapChain, float r, float g, float b, float a) {
	vklSetClearColor(swapChain->backBuffer->frameBuffer, r, g, b, a);
}

void rzvkPresent(VKCTX* ctx, VKSwapChain* swapChain) {
	vklPresent(swapChain->swapChain);
}

RZBuffer* rzvkAllocateBuffer(VKCTX* ctx, VKCommandQueue* queue, RZBufferCreateInfo* createInfo, void* data, size_t size) {
	VKBuffer* buffer = malloc(sizeof(VKBuffer));

	if (createInfo->type == RZ_BUFFER_TYPE_DYNAMIC) {
		vklCreateBuffer(ctx->device, &buffer->buffer, VK_FALSE, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		vklWriteToMemory(ctx->device, buffer->buffer->memory, data, size);
	} else if (createInfo->type == RZ_BUFFER_TYPE_STATIC) {
		vklCreateStagedBuffer(queue->devCon, &buffer->buffer, data, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	}
	
	return buffer;
}

void rzvkUpdateBuffer(VKCTX* ctx, VKBuffer* buffer, void* data, size_t size) {

}

void rzvkBindBuffer(VKCTX* ctx, VKCommandBuffer* cmdBuffer, VKBuffer* buffer) {
	VkDeviceSize offsets = 0;
	ctx->device->pvkCmdBindVertexBuffers(cmdBuffer->cmdBuffer, 0, 1, &buffer->buffer->buffer, &offsets);
}

void rzvkFreeBuffer(VKCTX* ctx, VKBuffer* buffer) {

}

RZShader* rzvkCreateShader(VKCTX* ctx, RZShaderCreateInfo* createInfo) {
	VKShader* shader = malloc(sizeof(VKShader));

	VKFrameBuffer* frameBuffer = (VKFrameBuffer*)createInfo->frameBuffer;
	
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
			bindings[i].binding = i;
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

	vklCreateShader(ctx->device, &shader->shader, &shaderCreateInfo);

	VKLPipelineCreateInfo pipelineCreateInfo;
	memset(&pipelineCreateInfo, 0, sizeof(VKLPipelineCreateInfo));
	pipelineCreateInfo.shader = shader->shader;
	pipelineCreateInfo.renderPass = frameBuffer->frameBuffer->renderPass;
	pipelineCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineCreateInfo.cullMode = VK_CULL_MODE_NONE;
	pipelineCreateInfo.extent.width = frameBuffer->frameBuffer->width;
	pipelineCreateInfo.extent.height = frameBuffer->frameBuffer->height;

	vklCreateGraphicsPipeline(ctx->device, &shader->pipeline, &pipelineCreateInfo);

	return shader;
}

void rzvkBindShader(VKCTX* ctx, VKCommandBuffer* cmdBuffer, VKShader* shader) {
	ctx->device->pvkCmdBindPipeline(cmdBuffer->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline->pipeline);
}

void rzvkDestroyShader(VKCTX* ctx, VKShader* shader) {

}

void rzvkDraw(VKCTX* ctx, VKCommandBuffer* cmdBuffer, uint32_t firstVertex, uint32_t vertexCount) {
	ctx->device->pvkCmdDraw(cmdBuffer->cmdBuffer, vertexCount, 1, firstVertex, 0);
}

RZUniform* rzvkCreateUniform(VKCTX* ctx, VKShader* shader) {
	VKUniform* uniform = malloc_c(sizeof(VKUniform));

	vklCreateUniformObject(ctx->device, &uniform->uniform, shader->shader);

	uniform->count = shader->descriptorCount;
	uniform->buffers = malloc_c(sizeof(VKLBuffer*) * uniform->count);
	uniform->types = malloc_c(sizeof(RZUniformType) * uniform->count);
	uniform->indexes = malloc_c(sizeof(uint32_t) * uniform->count);

	for (uint32_t i = 0; i < uniform->count;i++) {
		uniform->types[i] = shader->descriptors[i].type;
		uniform->indexes[i] = shader->descriptors[i].index;

		if (uniform->types[i] != RZ_UNIFORM_TYPE_SAMPLED_IMAGE) {
			vklCreateBuffer(ctx->device, &uniform->buffers[i], VK_FALSE, shader->descriptors[i].bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
			vklSetUniformBuffer(ctx->device, uniform->uniform, uniform->buffers[i], i);
		}

	}

	return uniform;
}

void rzvkBindUniform(VKCTX* ctx, VKCommandBuffer* cmdBuffer, VKShader* shader, VKUniform* uniform) {
	ctx->device->pvkCmdBindDescriptorSets(cmdBuffer->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		shader->pipeline->pipelineLayout, 0, 1, &uniform->uniform->descriptorSet, 0, NULL);
}

void rzvkUniformData(VKCTX* ctx, VKUniform* uniform, uint32_t index, void* data) {
	if (uniform->types[index] == RZ_UNIFORM_TYPE_SAMPLED_IMAGE) {
		VKTexture* texture = (VKTexture*)data;
		vklSetUniformTexture(ctx->device, uniform->uniform, texture->texture, index);
	} else {
		vklWriteToMemory(ctx->device, uniform->buffers[index]->memory, data, uniform->buffers[index]->size);
	}
}

void rzvkDestroyUniform(VKCTX* ctx, VKUniform* uniform) {
	
}

RZTexture* rzvkCreateTexture(VKCTX* ctx, VKCommandQueue* queue, RZTextureCreateInfo* createInfo) {
	VKTexture* texture = malloc_c(sizeof(VKTexture));

	VKLTextureCreateInfo textureCreateInfo;
	textureCreateInfo.width = createInfo->width;
	textureCreateInfo.height = createInfo->height;
	textureCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	textureCreateInfo.filter = VK_FILTER_NEAREST;
	
	RZComponentType type = createInfo->componentType;

	textureCreateInfo.colorCount = createInfo->componentsPerPixel;
	textureCreateInfo.colorSize = createInfo->bytesPerComponent;

	if (type == RZ_COMPONENT_TYPE_FLOAT_32) {
		if (createInfo->componentsPerPixel == 1) {
			textureCreateInfo.format = VK_FORMAT_R32_SFLOAT;
		} else if (createInfo->componentsPerPixel == 2) {
			textureCreateInfo.format = VK_FORMAT_R32G32_SFLOAT;
		} else if (createInfo->componentsPerPixel == 3) {
			textureCreateInfo.format = VK_FORMAT_R32G32B32_SFLOAT;
		} else if (createInfo->componentsPerPixel == 4) {
			textureCreateInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		}
	} else if (type == RZ_COMPONENT_TYPE_INT_32) {
		if (createInfo->componentsPerPixel == 1) {
			textureCreateInfo.format = VK_FORMAT_R32_UINT;
		} else if (createInfo->componentsPerPixel == 2) {
			textureCreateInfo.format = VK_FORMAT_R32G32_UINT;
		} else if (createInfo->componentsPerPixel == 3) {
			textureCreateInfo.format = VK_FORMAT_R32G32B32_UINT;
		} else if (createInfo->componentsPerPixel == 4) {
			textureCreateInfo.format = VK_FORMAT_R32G32B32A32_UINT;
		}
	} else if (type == RZ_COMPONENT_TYPE_INT_8) {
		if (createInfo->componentsPerPixel == 1) {
			textureCreateInfo.format = VK_FORMAT_R8_UNORM;
		} else if (createInfo->componentsPerPixel == 2) {
			textureCreateInfo.format = VK_FORMAT_R8G8_UNORM;
		} else if (createInfo->componentsPerPixel == 3) {
			textureCreateInfo.format = VK_FORMAT_R8G8B8_UNORM;
		} else if (createInfo->componentsPerPixel == 4) {
			textureCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		}
	}

	vklCreateStagedTexture(queue->devCon, &texture->texture, &textureCreateInfo, createInfo->data);

	return texture;
}

void rzvkDestroyTexture(VKCTX* ctx, RZTexture* texture) {
	
}

RZCommandBuffer* rzvkCreateCommandBuffer(VKCTX* ctx, VKCommandQueue* queue) {
	VKCommandBuffer* cmdBuffer = malloc(sizeof(VKCommandBuffer));
	vklAllocateCommandBuffer(queue->devCon, &cmdBuffer->cmdBuffer, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
	return cmdBuffer;
}

void rzvkStartCommandBuffer(VKCTX* ctx, VKCommandQueue* queue, VKCommandBuffer* cmdBuffer) {
	vklBeginCommandBuffer(ctx->device, cmdBuffer->cmdBuffer);
}

void rzvkStartRender(VKCTX* ctx, VKFrameBuffer* frameBuffer, VKCommandBuffer* cmdBuffer) {
	vklBeginRender(ctx->device, frameBuffer->frameBuffer, cmdBuffer->cmdBuffer);

	VkViewport viewport = { 0, 0, frameBuffer->frameBuffer->width, frameBuffer->frameBuffer->height, 0, 1 };
	VkRect2D scissor = {
		{ 0, 0 },
		{ frameBuffer->frameBuffer->width, frameBuffer->frameBuffer->height } };

	ctx->device->pvkCmdSetViewport(cmdBuffer->cmdBuffer, 0, 1, &viewport);
	ctx->device->pvkCmdSetScissor(cmdBuffer->cmdBuffer, 0, 1, &scissor);
}

void rzvkEndRender(VKCTX* ctx, VKFrameBuffer* frameBuffer, VKCommandBuffer* cmdBuffer) {
	vklEndRender(ctx->device, frameBuffer->frameBuffer, cmdBuffer->cmdBuffer);
}

void rzvkEndCommandBuffer(VKCTX* ctx, VKCommandBuffer* cmdBuffer) {
	vklEndCommandBuffer(ctx->device, cmdBuffer->cmdBuffer);
}

void rzvkExecuteCommandBuffer(VKCTX* ctx, VKCommandQueue* queue, VKCommandBuffer* cmdBuffer) {
	vklExecuteCommandBuffer(queue->devCon, cmdBuffer->cmdBuffer);
}

void rzvkLoadPFN(RZContext* ctx) {
	ctx->createDevice = rzvkInitContext;
	ctx->getBackBuffer = rzvkGetBackBuffer;
	ctx->setClearColor = rzvkSetClearColor;
	ctx->present = rzvkPresent;

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

	ctx->createTexture = rzvkCreateTexture;
	ctx->destroyTexture = rzvkDestroyTexture;

	ctx->createCommandBuffer = rzvkCreateCommandBuffer;
	ctx->startCommandBuffer = rzvkStartCommandBuffer;
	ctx->startRender = rzvkStartRender;
	ctx->endRender = rzvkEndRender;
	ctx->endCommandBuffer = rzvkEndCommandBuffer;
	ctx->executeCommandBuffer = rzvkExecuteCommandBuffer;
}
