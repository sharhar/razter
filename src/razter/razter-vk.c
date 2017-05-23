#include <razter/razter.h>
#include <malloc.h>
#include <VKL/VKL.h>

typedef struct VKCTX {
	VKLInstance* instance;
	VKLSurface* surface;
	VKLDevice* device;
	VKLDeviceGraphicsContext* devCon;
	VKLSwapChain* swapChain;
	VkCommandBuffer cmdBuffer;
} VKCTX;

void rzvkInit(RZRenderContext* ctx) {
	ctx->ctx = (VKCTX*)malloc(sizeof(VKCTX));
}

GLFWwindow* rzvkCreateWindow(RZRenderContext* ctx, int width, int height, const char* title) {
	VKCTX* vkCTX = (VKCTX*)ctx->ctx;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Razter Test", NULL, NULL);

	vklCreateInstance(&vkCTX->instance, NULL, 0, NULL);
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

void rzvkLoadPFN(RZRenderContext* ctx) {
	ctx->init = rzvkInit;
	ctx->createWindow = rzvkCreateWindow;
	ctx->clear = rzvkClear;
	ctx->setClearColor = rzvkSetClearColor;
	ctx->swap = rzvkSwap;
}

/*
typedef struct VKCTX {
	VKLInstance* instance;
	VKLSurface* surface;
	VKLDevice* device;
	VKLDeviceGraphicsContext* devCon;
	VKLSwapChain* swapChain;
	VkCommandBuffer cmdBuffer;
} VKCTX;

RZVKRenderContext::RZVKRenderContext() {
	m_ctx = (VKCTX*)malloc(sizeof(VKCTX));
}

GLFWwindow* RZVKRenderContext::createWindow(int width, int height, const char* title) {
	VKCTX* ctx = (VKCTX*)m_ctx;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Razter Test", NULL, NULL);

	vklCreateInstance(&ctx->instance, NULL, 0, NULL);
	vklCreateGLFWSurface(ctx->instance, &ctx->surface, window);
	VKLDeviceGraphicsContext** deviceContexts;
	vklCreateDevice(ctx->instance, &ctx->device, &ctx->surface, 1, &deviceContexts, 0, NULL);
	ctx->devCon = deviceContexts[0];
	vklCreateSwapChain(ctx->devCon, &ctx->swapChain, VK_TRUE);

	vklAllocateCommandBuffer(ctx->devCon, &ctx->cmdBuffer, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

	vklSetClearColor(ctx->swapChain, 1.0f, 0.0f, 1.0f, 1.0f);

	return window;
}

void RZVKRenderContext::clear() {
	VKCTX* ctx = (VKCTX*)m_ctx;

	vklClearScreen(ctx->swapChain);

	vklBeginRenderRecording(ctx->swapChain, ctx->cmdBuffer);
}

void RZVKRenderContext::swap() {
	VKCTX* ctx = (VKCTX*)m_ctx;

	vklEndRenderRecording(ctx->swapChain, ctx->cmdBuffer);

	vklRenderRecording(ctx->swapChain, ctx->cmdBuffer);

	vklSwapBuffers(ctx->swapChain);
}

*/