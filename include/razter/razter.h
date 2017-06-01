#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

typedef enum RZBool {
	RZ_FALSE = 0x00,
	RZ_TRUE = 0x01
} RZBool;

typedef enum RZPlatform {
	RZ_PLATFORM_OPENGL,
	RZ_PLATFORM_VULKAN
} RZPlatform;

typedef enum RZBufferUsage {
	RZ_BUFFER_USAGE_VERTEX = 0x01,
	RZ_BUFFER_USAGE_STORAGE = 0x02,
	RZ_BUFFER_USAGE_UNIFORM = 0x04
} RZBufferUsage;

typedef enum RZBufferType {
	RZ_BUFFER_TYPE_STATIC = 0x01,
	RZ_BUFFER_TYPE_DYNAMIC = 0x02
} RZBufferType;

typedef struct RZVertexAttributeDescription {
	size_t stride;
	size_t* offsets;
	uint32_t* sizes;
	uint32_t count;
} RZVertexAttributeDescription;

typedef struct RZBufferCreateInfo {
	RZBufferType type;
	RZBufferUsage usage;
	RZVertexAttributeDescription* vertexAttribDesc;
} RZBufferCreateInfo;

typedef struct RZShaderCreateInfo {
	char* vertData;
	size_t vertSize;
	char* fragData;
	size_t fragSize;
	RZVertexAttributeDescription* vertexAttribDesc;
	RZBool isPath;
} RZShaderCreateInfo;

typedef void RZBuffer;
typedef void RZShader;
typedef void RZUniform;

struct RZRenderContext;
typedef struct RZRenderContext RZRenderContext;

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

	RZShader* (*createShader)(RZRenderContext* ctx, RZShaderCreateInfo* createInfo);
	void (*bindShader)(RZRenderContext* ctx, RZShader* shader);
	void (*destroyShader)(RZRenderContext* ctx, RZShader* shader);

	void(*draw)(RZRenderContext* ctx, uint32_t firstVertex, uint32_t vertexCount);
} RZRenderContext;

void rzglLoadPFN(RZRenderContext* ctx);
void rzvkLoadPFN(RZRenderContext* ctx);

RZRenderContext* rzCreateRenderContext(RZPlatform type);

GLFWwindow* rzCreateWindow(RZRenderContext* ctx, int width, int height, const char* title);
void rzClear(RZRenderContext* ctx);
void rzSetClearColor(RZRenderContext* ctx, float r, float g, float b, float a);
void rzSwap(RZRenderContext* ctx);

RZBuffer* rzAllocateBuffer(RZRenderContext* ctx, RZBufferCreateInfo* createInfo, void* data, size_t size);
void rzUpdateBuffer(RZRenderContext* ctx, RZBuffer* buffer, void* data, size_t size);
void rzBindBuffer(RZRenderContext* ctx, RZBuffer* buffer);
void rzFreeBuffer(RZRenderContext* ctx, RZBuffer* buffer);

RZShader* rzCreateShader(RZRenderContext* ctx, RZShaderCreateInfo* createInfo);
void rzBindShader(RZRenderContext* ctx, RZShader* shader);
void rzDestroyShader(RZRenderContext* ctx, RZShader* shader);

void rzDraw(RZRenderContext* ctx, uint32_t firstVertex, uint32_t vertexCount);

char* rzReadFileFromPath(char *filename, size_t* size);

#ifdef __cplusplus
}
#endif