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
	uint32_t count;
	RZUniformType* types;
	uint32_t* indexes;
} VKUniform;

typedef struct VKTexture {
	VKLTexture* texture;
} VKTexture;

void rzvkInit(RZRenderContext* ctx) {
	ctx->ctx = (VKCTX*)malloc(sizeof(VKCTX));
}

GLFWwindow* rzvkCreateWindow(VKCTX* ctx, int width, int height, const char* title) {
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Razter Test", NULL, NULL);

	vklCreateInstance(&ctx->instance, NULL, VK_TRUE, NULL);
	vklCreateGLFWSurface(ctx->instance, &ctx->surface, window);
	VKLDeviceGraphicsContext** deviceContexts;
	vklCreateDevice(ctx->instance, &ctx->device, &ctx->surface, 1, &deviceContexts, 0, NULL);
	ctx->devCon = deviceContexts[0];
	vklCreateSwapChain(ctx->devCon, &ctx->swapChain, VK_TRUE);

	vklAllocateCommandBuffer(ctx->devCon, &ctx->cmdBuffer, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

	return window;
}

void rzvkClear(VKCTX* ctx) {
	vklClearScreen(ctx->swapChain);
	vklBeginRenderRecording(ctx->swapChain, ctx->cmdBuffer);

	VkViewport viewport = { 0, 0, ctx->swapChain->width, ctx->swapChain->height, 0, 1 };
	VkRect2D scissor = {
		{0, 0},
		{ ctx->swapChain->width, ctx->swapChain->height} };
	
	ctx->device->pvkCmdSetViewport(ctx->cmdBuffer, 0, 1, &viewport);
	ctx->device->pvkCmdSetScissor(ctx->cmdBuffer, 0, 1, &scissor);
}

void rzvkSetClearColor(VKCTX* ctx, float r, float g, float b, float a) {
	vklSetClearColor(ctx->swapChain, r, g, b, a);
}

void rzvkSwap(VKCTX* ctx) {
	vklEndRenderRecording(ctx->swapChain, ctx->cmdBuffer);
	vklRenderRecording(ctx->swapChain, ctx->cmdBuffer);
	vklSwapBuffers(ctx->swapChain);
}

RZBuffer* rzvkAllocateBuffer(VKCTX* ctx, RZBufferCreateInfo* createInfo, void* data, size_t size) {
	VKBuffer* buffer = malloc(sizeof(VKBuffer));

	if (createInfo->type == RZ_BUFFER_TYPE_DYNAMIC) {
		vklCreateBuffer(ctx->device, &buffer->buffer, VK_FALSE, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		vklWriteToMemory(ctx->device, buffer->buffer->memory, data, size);
	} else if (createInfo->type == RZ_BUFFER_TYPE_STATIC) {
		vklCreateStagedBuffer(ctx->devCon, &buffer->buffer, data, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	}
	
	return buffer;
}

void rzvkUpdateBuffer(VKCTX* ctx, VKBuffer* buffer, void* data, size_t size) {

}

void rzvkBindBuffer(VKCTX* ctx, VKBuffer* buffer) {
	VkDeviceSize offsets = 0;
	ctx->device->pvkCmdBindVertexBuffers(ctx->cmdBuffer, 0, 1, &buffer->buffer->buffer, &offsets);
}

void rzvkFreeBuffer(VKCTX* ctx, VKBuffer* buffer) {

}

RZShader* rzvkCreateShader(VKCTX* ctx, RZShaderCreateInfo* createInfo) {
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

	vklCreateShader(ctx->device, &shader->shader, &shaderCreateInfo);

	VKLPipelineCreateInfo pipelineCreateInfo;
	memset(&pipelineCreateInfo, 0, sizeof(VKLPipelineCreateInfo));
	pipelineCreateInfo.shader = shader->shader;
	pipelineCreateInfo.renderPass = ctx->swapChain->renderPass;
	pipelineCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineCreateInfo.cullMode = VK_CULL_MODE_NONE;
	pipelineCreateInfo.extent.width = ctx->swapChain->width;
	pipelineCreateInfo.extent.height = ctx->swapChain->height;

	vklCreateGraphicsPipeline(ctx->device, &shader->pipeline, &pipelineCreateInfo);

	return shader;
}

void rzvkBindShader(VKCTX* ctx, VKShader* shader) {
	ctx->device->pvkCmdBindPipeline(ctx->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline->pipeline);
}

void rzvkDestroyShader(VKCTX* ctx, VKShader* shader) {

}

void rzvkDraw(VKCTX* ctx, uint32_t firstVertex, uint32_t vertexCount) {
	ctx->device->pvkCmdDraw(ctx->cmdBuffer, vertexCount, 1, firstVertex, 0);
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
			vklSetUniformBuffer(ctx->device, uniform->uniform, uniform->buffers[i], shader->descriptors[i].index);
		}

	}

	return uniform;
}

void rzvkBindUniform(VKCTX* ctx, VKShader* shader, VKUniform* uniform) {
	ctx->device->pvkCmdBindDescriptorSets(ctx->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		shader->pipeline->pipelineLayout, 0, 1, &uniform->uniform->descriptorSet, 0, NULL);
}

void rzvkUniformData(VKCTX* ctx, VKUniform* uniform, uint32_t index, void* data) {
	if (uniform->types[index] == RZ_UNIFORM_TYPE_SAMPLED_IMAGE) {
		VKTexture* texture = (VKTexture*)data;
		vklSetUniformTexture(ctx->device, uniform->uniform, texture->texture, uniform->indexes[index]);
	} else {
		vklWriteToMemory(ctx->device, uniform->buffers[index]->memory, data, uniform->buffers[index]->size);
	}
}

void rzvkDestroyUniform(VKCTX* ctx, VKUniform* uniform) {
	
}

RZTexture* rzvkCreateTexture(VKCTX* ctx, RZTextureCreateInfo* createInfo) {
	VKTexture* texture = malloc_c(sizeof(VKTexture));

	VKLTextureCreateInfo textureCreateInfo;
	textureCreateInfo.width = createInfo->width;
	textureCreateInfo.height = createInfo->height;
	textureCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	textureCreateInfo.filter = VK_FILTER_NEAREST;
	
	/*
	RZColorFormat format = createInfo->colorFormat;
	RZColorSize size = createInfo->colorSize;

	if (format == RZ_COLOR_FORMAT_R) {
		if (size == RZ_COLOR_SIZE_FLOAT_32) {
			textureCreateInfo.format = VK_FORMAT_R32_SFLOAT;
			textureCreateInfo.colorSize = sizeof(float);
			textureCreateInfo.colorCount = 1;
		}
		else if (size == RZ_COLOR_SIZE_INT_32) {
			textureCreateInfo.format = VK_FORMAT_R32_UINT;
			textureCreateInfo.colorSize = sizeof(uint32_t);
			textureCreateInfo.colorCount = 1;
		}
		else if (size == RZ_COLOR_SIZE_INT_8) {
			textureCreateInfo.format = VK_FORMAT_R8_UNORM;
			textureCreateInfo.colorSize = sizeof(uint8_t);
			textureCreateInfo.colorCount = 1;
		}
	} else if (format == RZ_COLOR_FORMAT_RG) {
		if (size == RZ_COLOR_SIZE_FLOAT_32) {
			textureCreateInfo.format = VK_FORMAT_R32G32_SFLOAT;
			textureCreateInfo.colorSize = sizeof(float);
			textureCreateInfo.colorCount = 2;
		}
		else if (size == RZ_COLOR_SIZE_INT_32) {
			textureCreateInfo.format = VK_FORMAT_R32G32_UINT;
			textureCreateInfo.colorSize = sizeof(uint32_t);
			textureCreateInfo.colorCount = 2;
		}
		else if (size == RZ_COLOR_SIZE_INT_8) {
			textureCreateInfo.format = VK_FORMAT_R8G8_UNORM;
			textureCreateInfo.colorSize = sizeof(uint8_t);
			textureCreateInfo.colorCount = 2;
		}
	} else if (format == RZ_COLOR_FORMAT_RGB) {
		if (size == RZ_COLOR_SIZE_FLOAT_32) {
			textureCreateInfo.format = VK_FORMAT_R32G32B32_SFLOAT;
			textureCreateInfo.colorSize = sizeof(float);
			textureCreateInfo.colorCount = 3;
		}
		else if (size == RZ_COLOR_SIZE_INT_32) {
			textureCreateInfo.format = VK_FORMAT_R32G32B32_SINT;
			textureCreateInfo.colorSize = sizeof(uint32_t);
			textureCreateInfo.colorCount = 3;
		}
		else if (size == RZ_COLOR_SIZE_INT_8) {
			textureCreateInfo.format = VK_FORMAT_R8G8B8_UNORM;
			textureCreateInfo.colorSize = sizeof(uint8_t);
			textureCreateInfo.colorCount = 3;
		}
	} else if (format == RZ_COLOR_FORMAT_RGBA) {
		if (size == RZ_COLOR_SIZE_FLOAT_32) {
			textureCreateInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			textureCreateInfo.colorSize = sizeof(float);
			textureCreateInfo.colorCount = 4;
		} else if (size == RZ_COLOR_SIZE_INT_32) {
			textureCreateInfo.format = VK_FORMAT_R32G32B32A32_UINT;
			textureCreateInfo.colorSize = sizeof(uint32_t);
			textureCreateInfo.colorCount = 4;
		} else if (size == RZ_COLOR_SIZE_INT_8) {
			textureCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
			textureCreateInfo.colorSize = sizeof(uint8_t);
			textureCreateInfo.colorCount = 4;
		}
	}
	*/
	

	vklCreateStagedTexture(ctx->devCon, &texture->texture, &textureCreateInfo, createInfo->data);

	return texture;
}

void rzvkDestroyTexture(VKCTX* ctx, RZTexture* texture) {
	
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

	ctx->createTexture = rzvkCreateTexture;
	ctx->destroyTexture = rzvkDestroyTexture;
}
