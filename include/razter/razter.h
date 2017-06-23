#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdlib.h>

typedef void RZDevice;
typedef void RZSwapChain;
typedef void RZBuffer;
typedef void RZShader;
typedef void RZUniform;
typedef void RZTexture;
typedef void RZCommandQueue;
typedef void RZCommandBuffer;
typedef void RZFrameBuffer;

typedef enum RZBool {
	RZ_FALSE = 0x00,
	RZ_TRUE = 0x01
} RZBool;

typedef enum RZAPI {
	RZ_API_METAL,
	RZ_API_VULKAN
} RZAPI;

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
	RZFrameBuffer* frameBuffer;
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

typedef struct RZContext {
	void (*createDevice)(RZDevice** pDevice, GLFWwindow* window, RZSwapChain** pSwapChain, RZBool debug, uint32_t queueCount, RZCommandQueue*** pQueues);
	RZFrameBuffer* (*getBackBuffer)(RZSwapChain* swapChain);
	void (*setClearColor)(RZSwapChain* ctx, float r, float g, float b, float a);
	void (*present)(RZDevice* ctx, RZSwapChain* swapChain);

	RZBuffer* (*allocBuffer)(RZDevice* ctx, RZCommandQueue* queue, RZBufferCreateInfo* createInfo, void* data, size_t size);
	void (*updateBuffer)(RZDevice* ctx, RZBuffer* buffer, void* data, size_t size);
	void (*bindBuffer)(RZDevice* ctx, RZCommandBuffer* cmdBuffer, RZBuffer* buffer);
	void (*freeBuffer)(RZDevice* ctx, RZBuffer* buffer);

	RZShader* (*createShader)(RZDevice* ctx, RZShaderCreateInfo* createInfo);
	void (*bindShader)(RZDevice* ctx, RZCommandBuffer* cmdBuffer, RZShader* shader);
	void (*destroyShader)(RZDevice* ctx, RZShader* shader);

	void(*draw)(RZDevice* ctx, RZCommandBuffer* cmdBuffer, uint32_t firstVertex, uint32_t vertexCount);

	RZUniform* (*createUniform)(RZDevice* ctx, RZShader* shader);
	void (*bindUniform)(RZDevice* ctx, RZCommandBuffer* cmdBuffer, RZShader* shader, RZUniform* uniform);
	void (*uniformData)(RZDevice* ctx, RZUniform* uniform, uint32_t index, void* data);
	void (*destroyUniform)(RZDevice* ctx, RZUniform* uniform);

	RZTexture* (*createTexture)(RZDevice* ctx, RZCommandQueue* queue, RZTextureCreateInfo* createInfo);
	void(*destroyTexture)(RZDevice* ctx, RZTexture* texture);

	RZCommandBuffer* (*createCommandBuffer)(RZDevice* ctx, RZCommandQueue* queue);
	void(*startCommandBuffer)(RZDevice* ctx, RZCommandQueue* queue, RZCommandBuffer* cmdBuffer);
	void(*startRender)(RZDevice* ctx, RZFrameBuffer* frameBuffer, RZCommandBuffer* cmdBuffer);
	void(*endRender)(RZDevice* ctx, RZFrameBuffer* frameBuffer, RZCommandBuffer* cmdBuffer);
	void(*endCommandBuffer)(RZDevice* ctx, RZCommandBuffer* cmdBuffer);
	void(*executeCommandBuffer)(RZDevice* ctx, RZCommandQueue* queue, RZCommandBuffer* cmdBuffer);
} RZContext;

void rzvkLoadPFN(RZContext* ctx);
	
#ifdef __APPLE__
void rzmtLoadPFN(RZContext* ctx);
#endif

RZContext* rzCreateContext(RZAPI type);

char* rzReadFileFromPath(char *filename, size_t* size);

#ifdef __cplusplus
}
#endif
