#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

struct RZRenderContext;
typedef struct RZRenderContext RZRenderContext;
typedef struct RZRenderContext {
	void* ctx;
	GLFWwindow* (*createWindow)(RZRenderContext* ctx, int width, int height, const char* title);
	void (*init)(RZRenderContext* ctx);
	void (*setClearColor)(RZRenderContext* ctx, float r, float g, float b, float a);
	void (*clear)(RZRenderContext* ctx);
	void (*swap)(RZRenderContext* ctx);
} RZRenderContext;

#define RZ_RC_GL 1
#define RZ_RC_VK 2
#define RZ_RC_DX 3

RZRenderContext* rzCreateRenderContext(int type);

GLFWwindow* rzCreateWindow(RZRenderContext* ctx, int width, int height, const char* title);
void rzClear(RZRenderContext* ctx);
void rzSetClearColor(RZRenderContext* ctx, float r, float g, float b, float a);
void rzSwap(RZRenderContext* ctx);

void rzglLoadPFN(RZRenderContext* ctx);
void rzvkLoadPFN(RZRenderContext* ctx);
void rzdxLoadPFN(RZRenderContext* ctx);

#ifdef __cplusplus
}
#endif