#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

typedef enum RZPlatform {
	RZ_PLATFORM_OPENGL,
	RZ_PLATFORM_VULKAN
	
#ifdef __APPLE__
	,RZ_PLATFORM_METAL
#endif

} RZPlatform;

struct RZRenderContext;
typedef struct RZRenderContext RZRenderContext;
	
typedef enum RZBufferUsage {
	RZ_BUFFER_USAGE_STATIC,
	RZ_BUFFER_USAGE_DYNAMIC
} RZBufferUsage;
	
typedef struct RZBufferCreateInfo {
	RZBufferUsage usage;
} RZBufferCreateInfo;
	
typedef struct RZBuffer {
	void* data;
	size_t size;
} RZBuffer;


typedef struct RZRenderContext {
	void* ctx;
	GLFWwindow* (*createWindow)(RZRenderContext* ctx, int width, int height, const char* title);
	void (*init)(RZRenderContext* ctx);
	void (*setClearColor)(RZRenderContext* ctx, float r, float g, float b, float a);
	void (*clear)(RZRenderContext* ctx);
	void (*swap)(RZRenderContext* ctx);
	RZBuffer* (*allocBuffer)(RZRenderContext* ctx, RZBufferCreateInfo* createInfo, void* data, size_t size);
	void (*updateBuffer)(RZRenderContext* ctx, RZBuffer* buffer, void* data, size_t size);
	void (*bindBuffer)(RZRenderContext* ctx, RZBuffer* buffer);
	void (*freeBuffer)(RZRenderContext* ctx, RZBuffer* buffer);
} RZRenderContext;

void rzglLoadPFN(RZRenderContext* ctx);
void rzvkLoadPFN(RZRenderContext* ctx);
	
#ifdef __APPLE__
void rzmtlLoadPFN(RZRenderContext* ctx);
#endif

RZRenderContext* rzCreateRenderContext(RZPlatform type);

GLFWwindow* rzCreateWindow(RZRenderContext* ctx, int width, int height, const char* title);
void rzClear(RZRenderContext* ctx);
void rzSetClearColor(RZRenderContext* ctx, float r, float g, float b, float a);
void rzSwap(RZRenderContext* ctx);

RZBuffer* rzAllocateBuffer(RZRenderContext* ctx, RZBufferCreateInfo* createInfo, void* data, size_t size);
void rzUpdateBuffer(RZRenderContext* ctx, RZBuffer* buffer, void* data, size_t size);
void rzBindBuffer(RZRenderContext* ctx, RZBuffer* buffer);
void rzFreeBuffer(RZRenderContext* ctx, RZBuffer* buffer);

#ifdef __cplusplus
}
#endif
