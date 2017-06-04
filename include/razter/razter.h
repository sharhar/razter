#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdlib.h>

typedef enum RZBool {
	RZ_FALSE = 0x00,
	RZ_TRUE = 0x01
} RZBool;

typedef enum RZPlatform {
	RZ_PLATFORM_METAL,
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

typedef enum RZUniformType {
	RZ_UNIFORM_TYPE_BUFFER			= 0x01,
	RZ_UNIFORM_TYPE_SAMPLED_IMAGE	= 0x02
} RZUniformType;

typedef enum RZUniformStage {
	RZ_UNIFORM_STAGE_VERTEX = 0x01,
	RZ_UNIFORM_STAGE_FRAGMENT = 0x02
} RZUniformStage;

typedef struct RZUniformDescriptor {
	RZUniformType type;
	RZUniformStage stage;
	uint32_t index;
	const char* name;
	size_t bufferSize;
} RZUniformDescriptor;

typedef struct RZShaderCreateInfo {
	char* vertData;
	size_t vertSize;
	char* fragData;
	size_t fragSize;
	char* vertFunction;
	char* fragFunction;
	RZVertexAttributeDescription* vertexAttribDesc;
	RZBool isPath;
	RZUniformDescriptor* descriptors;
	uint32_t descriptorCount;
} RZShaderCreateInfo;

typedef enum RZComponentType {
	RZ_COMPONENT_TYPE_FLOAT_32 = 0x01,
	RZ_COMPONENT_TYPE_INT_32 = 0x02,
	RZ_COMPONENT_TYPE_INT_8 = 0x03,
} RZComponentType;

typedef struct RZTextureCreateInfo {
	uint32_t width;
	uint32_t height;
	RZComponentType componentType;
	size_t bytesPerComponent;
	size_t componentsPerPixel;
	void* data;
} RZTextureCreateInfo;

typedef void RZInternalContext;
typedef void RZBuffer;
typedef void RZShader;
typedef void RZUniform;
typedef void RZTexture;
	
struct RZRenderContext;
typedef struct RZRenderContext RZRenderContext;

typedef struct RZRenderContext {
	RZInternalContext* ctx;

	void(*init)(RZRenderContext* ctx);

	GLFWwindow* (*createWindow)(RZInternalContext* ctx, int width, int height, const char* title);
	void (*setClearColor)(RZInternalContext* ctx, float r, float g, float b, float a);
	void (*clear)(RZInternalContext* ctx);
	void (*swap)(RZInternalContext* ctx);

	RZBuffer* (*allocBuffer)(RZInternalContext* ctx, RZBufferCreateInfo* createInfo, void* data, size_t size);
	void (*updateBuffer)(RZInternalContext* ctx, RZBuffer* buffer, void* data, size_t size);
	void (*bindBuffer)(RZInternalContext* ctx, RZBuffer* buffer);
	void (*freeBuffer)(RZInternalContext* ctx, RZBuffer* buffer);

	RZShader* (*createShader)(RZInternalContext* ctx, RZShaderCreateInfo* createInfo);
	void (*bindShader)(RZInternalContext* ctx, RZShader* shader);
	void (*destroyShader)(RZInternalContext* ctx, RZShader* shader);

	void(*draw)(RZInternalContext* ctx, uint32_t firstVertex, uint32_t vertexCount);

	RZUniform* (*createUniform)(RZInternalContext* ctx, RZShader* shader);
	void (*bindUniform)(RZInternalContext* ctx, RZShader* shader, RZUniform* uniform);
	void (*uniformData)(RZInternalContext* ctx, RZUniform* uniform, uint32_t index, void* data);
	void (*destroyUniform)(RZInternalContext* ctx, RZUniform* uniform);

	RZTexture* (*createTexture)(RZInternalContext* ctx, RZTextureCreateInfo* createInfo);
	void(*destroyTexture)(RZInternalContext* ctx, RZTexture* texture);
} RZRenderContext;

void rzvkLoadPFN(RZRenderContext* ctx);
	
#ifdef __APPLE__
void rzmtLoadPFN(RZRenderContext* ctx);
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

RZShader* rzCreateShader(RZRenderContext* ctx, RZShaderCreateInfo* createInfo);
void rzBindShader(RZRenderContext* ctx, RZShader* shader);
void rzDestroyShader(RZRenderContext* ctx, RZShader* shader);

void rzDraw(RZRenderContext* ctx, uint32_t firstVertex, uint32_t vertexCount);

RZUniform* rzCreateUniform(RZRenderContext* ctx, RZShader* shader);
void rzBindUniform(RZRenderContext* ctx, RZShader* shader, RZUniform* uniform);
void rzUniformData(RZRenderContext* ctx, RZUniform* uniform, uint32_t index, void* data);
void rzDestroyUniform(RZRenderContext* ctx, RZUniform* uniform);

RZTexture* rzCreateTexture(RZRenderContext* ctx, RZTextureCreateInfo* createInfo);
void rzDestroyTexture(RZRenderContext* ctx, RZTexture* texture);

char* rzReadFileFromPath(char *filename, size_t* size);

#ifdef __cplusplus
}
#endif
