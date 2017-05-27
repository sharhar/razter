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

typedef struct VKBuffer {
	VKLBuffer* buffer;
} VKBuffer;

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

	VkViewport viewport = { 0, 0, vkCTX->swapChain->width, vkCTX->swapChain->height, 0, 1 };
	VkRect2D scissor = { 0, 0, vkCTX->swapChain->width, vkCTX->swapChain->height };
	
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

}

void rzvkBindShader(RZRenderContext* ctx, RZShader* shader) {

}

void rzvkDestroyShader(RZRenderContext* ctx, RZShader* shader) {

}

void rzvkDraw(RZRenderContext* ctx, uint32_t firstVertex, uint32_t vertexCount) {
	
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
}