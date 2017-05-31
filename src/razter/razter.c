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

	if (type == RZ_PLATFORM_OPENGL) {
		rzglLoadPFN(ctx);
	} else if (type == RZ_PLATFORM_VULKAN) {
		if (glfwVulkanSupported()) {
			rzvkLoadPFN(ctx);
		} else {
			return NULL;
		}
	}
#ifdef __APPLE__
	else if (type == RZ_PLATFORM_METAL) {
		rzmtlLoadPFN(ctx);
	}
#endif
	else {
		return NULL;
	}

	ctx->init(ctx);
	return ctx;
}

GLFWwindow* rzCreateWindow(RZRenderContext* ctx, int width, int height, const char* title) {
	GLFWwindow* window = ctx->createWindow(ctx, width, height, title);
	ctx->setClearColor(ctx, 0.0f, 0.0f, 0.0f, 1.0f);
	return window;
}

void rzClear(RZRenderContext* ctx) {
	ctx->clear(ctx);
}

void rzSetClearColor(RZRenderContext* ctx, float r, float g, float b, float a) {
	ctx->setClearColor(ctx, r, g, b, a);
}

void rzSwap(RZRenderContext* ctx) {
	ctx->swap(ctx);
}

RZBuffer* rzAllocateBuffer(RZRenderContext* ctx, RZBufferCreateInfo* createInfo, void* data, size_t size) {
	return ctx->allocBuffer(ctx, createInfo, data, size);
}

void rzUpdateBuffer(RZRenderContext* ctx, RZBuffer* buffer, void* data, size_t size) {
	ctx->updateBuffer(ctx, buffer, data, size);
}

void rzBindBuffer(RZRenderContext* ctx, RZBuffer* buffer) {
	ctx->bindBuffer(ctx, buffer);
}

void rzFreeBuffer(RZRenderContext* ctx, RZBuffer* buffer) {
	ctx->freeBuffer(ctx, buffer);
}

RZShader* rzCreateShader(RZRenderContext* ctx, RZShaderCreateInfo* createInfo) {
	return ctx->createShader(ctx, createInfo);
}

void rzBindShader(RZRenderContext* ctx, RZShader* shader) {
	ctx->bindShader(ctx, shader);
}

void rzDestroyShader(RZRenderContext* ctx, RZShader* shader) {
	ctx->destroyShader(ctx, shader);
}

void rzDraw(RZRenderContext* ctx, uint32_t firstVertex, uint32_t vertexCount) {
	ctx->draw(ctx, firstVertex, vertexCount);
}
