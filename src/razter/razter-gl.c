#include <glad/glad.h>
#include <razter/razter.h>
#include <malloc.h>

typedef struct GLCTX {
	GLFWwindow* window;
} GLCTX;

void rzglInit(RZRenderContext* ctx) {
	ctx->ctx = (GLCTX*)malloc(sizeof(GLCTX));
}

GLFWwindow* rzglCreateWindow(RZRenderContext* ctx, int width, int height, const char* title) {
	GLCTX* glCTX = (GLCTX*)ctx->ctx;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	glfwMakeContextCurrent(window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glCTX->window = window;

	return window;
}

void rzglClear(RZRenderContext* ctx) {
	glClear(GL_COLOR_BUFFER_BIT);
}

void rzglSetClearColor(RZRenderContext* ctx, float r, float g, float b, float a) {
	glClearColor(r, g, b, a);
}

void rzglSwap(RZRenderContext* ctx) {
	GLCTX* glCTX = (GLCTX*)ctx->ctx;
	glfwSwapBuffers(glCTX->window);
}

void rzglLoadPFN(RZRenderContext* ctx) {
	ctx->init = rzglInit;
	ctx->createWindow = rzglCreateWindow;
	ctx->clear = rzglClear;
	ctx->setClearColor = rzglSetClearColor;
	ctx->swap = rzglSwap;
}