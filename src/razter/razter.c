#include <razter/razter.h>
#include <stdio.h>

char* rzReadFileFromPath(char *filename, size_t* size) {
	char *buffer = NULL;
	size_t string_size, read_size;
	FILE *handler = fopen(filename, "rb");

	if (handler) {
		fseek(handler, 0, SEEK_END);
		string_size = ftell(handler);
		rewind(handler);

		buffer = (char*)malloc(sizeof(char) * (string_size + 1));

		read_size = fread(buffer, sizeof(char), string_size, handler);

		buffer[string_size] = '\0';

		if (string_size != read_size) {
			printf("Error occured while reading file!\nstring_size = %zu\nread_size = %zu\n\n", string_size, read_size);
			free(buffer);
			buffer = NULL;
		}

		*size = read_size;

		fclose(handler);
	}
	else {
		printf("Did not find file!\n");
	}

	return buffer;
}

RZRenderContext* rzCreateRenderContext(RZPlatform type) {
	RZRenderContext* ctx = malloc(sizeof(RZRenderContext));

	if (type == RZ_PLATFORM_VULKAN) {
		if (glfwVulkanSupported()) {
			rzvkLoadPFN(ctx);
		} else {
			return NULL;
		}
	}
#ifdef __APPLE__
	else if (type == RZ_PLATFORM_METAL) {
		rzmtLoadPFN(ctx);
	}
#endif
	else {
		return NULL;
	}

	return ctx;
}

void rzInitContext(RZRenderContext* ctx, GLFWwindow* window, RZBool debug, uint32_t queueCount, RZCommandQueue*** pQueues) {
	ctx->initContext(ctx, window, debug, queueCount, pQueues);
}

void rzClear(RZRenderContext* ctx, RZCommandQueue* queue, RZCommandBuffer* cmdBuffer) {
	ctx->clear(ctx->ctx, queue, cmdBuffer);
}

void rzSetClearColor(RZRenderContext* ctx, float r, float g, float b, float a) {
	ctx->setClearColor(ctx->ctx, r, g, b, a);
}

void rzSwap(RZRenderContext* ctx, RZCommandBuffer* cmdBuffer) {
	ctx->swap(ctx->ctx, cmdBuffer);
}

RZBuffer* rzAllocateBuffer(RZRenderContext* ctx, RZCommandQueue* queue, RZBufferCreateInfo* createInfo, void* data, size_t size) {
	return ctx->allocBuffer(ctx->ctx, queue, createInfo, data, size);
}

void rzUpdateBuffer(RZRenderContext* ctx, RZBuffer* buffer, void* data, size_t size) {
	ctx->updateBuffer(ctx->ctx, buffer, data, size);
}

void rzBindBuffer(RZRenderContext* ctx, RZCommandBuffer* cmdBuffer, RZBuffer* buffer) {
	ctx->bindBuffer(ctx->ctx, cmdBuffer, buffer);
}

void rzFreeBuffer(RZRenderContext* ctx, RZBuffer* buffer) {
	ctx->freeBuffer(ctx->ctx, buffer);
}

RZShader* rzCreateShader(RZRenderContext* ctx, RZShaderCreateInfo* createInfo) {
	return ctx->createShader(ctx->ctx, createInfo);
}

void rzBindShader(RZRenderContext* ctx, RZCommandBuffer* cmdBuffer, RZShader* shader) {
	ctx->bindShader(ctx->ctx, cmdBuffer, shader);
}

void rzDestroyShader(RZRenderContext* ctx, RZShader* shader) {
	ctx->destroyShader(ctx->ctx, shader);
}

void rzDraw(RZRenderContext* ctx, RZCommandBuffer* cmdBuffer, uint32_t firstVertex, uint32_t vertexCount) {
	ctx->draw(ctx->ctx, cmdBuffer, firstVertex, vertexCount);
}

RZUniform* rzCreateUniform(RZRenderContext* ctx, RZShader* shader) {
	return ctx->createUniform(ctx->ctx, shader);
}

void rzBindUniform(RZRenderContext* ctx, RZCommandBuffer* cmdBuffer, RZShader* shader, RZUniform* uniform) {
	ctx->bindUniform(ctx->ctx, cmdBuffer, shader, uniform);
}

void rzUniformData(RZRenderContext* ctx, RZUniform* uniform, uint32_t index, void* data) {
	ctx->uniformData(ctx->ctx, uniform, index, data);
}

void rzDestroyUniform(RZRenderContext* ctx, RZUniform* uniform) {
	ctx->destroyUniform(ctx->ctx, uniform);
}

RZTexture* rzCreateTexture(RZRenderContext* ctx, RZCommandQueue* queue, RZTextureCreateInfo* createInfo) {
	return ctx->createTexture(ctx->ctx, queue, createInfo);
}

void rzDestroyTexture(RZRenderContext* ctx, RZTexture* texture) {
	return ctx->destroyTexture(ctx->ctx, texture);
}

RZCommandBuffer* rzCreateCommandBuffer(RZRenderContext* ctx, RZCommandQueue* queue) {
	return ctx->createCommandBuffer(ctx->ctx, queue);
}

void rzStartRecording(RZRenderContext* ctx, RZCommandBuffer* cmdBuffer) {
	ctx->startRecording(ctx->ctx, cmdBuffer);
}

void rzStartRenderRecording(RZRenderContext* ctx, RZCommandBuffer* cmdBuffer) {
	ctx->startRenderRecording(ctx->ctx, cmdBuffer);
}

void rzEndRenderRecording(RZRenderContext* ctx, RZCommandBuffer* cmdBuffer) {
	ctx->endRenderRecording(ctx->ctx, cmdBuffer);
}

void rzEndRecording(RZRenderContext* ctx, RZCommandBuffer* cmdBuffer) {
	ctx->endRecording(ctx->ctx, cmdBuffer);
}

void rzSubmitCommandBuffer(RZRenderContext* ctx, RZCommandQueue* queue, RZCommandBuffer* cmdBuffer) {
	ctx->submitCommandBuffer(ctx->ctx, queue, cmdBuffer);
}
