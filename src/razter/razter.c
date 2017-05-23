#include <razter/razter.h>
#include <malloc.h>

RZRenderContext* rzCreateRenderContext(int type) {
	RZRenderContext* ctx = malloc(sizeof(RZRenderContext));

	if (type == RZ_RC_GL) {
		rzglLoadPFN(ctx);
	} else if (type == RZ_RC_VK) {
		if (glfwVulkanSupported()) {
			rzvkLoadPFN(ctx);
		} else {
			return NULL;
		}
	} else {
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
