#include <razter/razter.h>
#include <memory.h>

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
