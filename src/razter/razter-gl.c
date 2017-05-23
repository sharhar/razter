#include <glad/glad.h>
#include <razter/razter.h>
#include <malloc.h>

typedef struct GLCTX {
	GLFWwindow* window;
} GLCTX;

typedef struct GLBuffer {
	GLuint vao;
	GLuint vbo;
} GLBuffer;

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

RZBuffer* rzglAllocateBuffer(RZRenderContext* ctx, RZBufferCreateInfo* createInfo, void* data, size_t size) {
	RZBuffer* buffer = malloc(sizeof(RZBuffer));

	buffer->size = size;
	buffer->data = malloc(sizeof(GLBuffer));
	GLBuffer* glBuff = (GLBuffer*)buffer->data;

	glGenVertexArrays(1, &glBuff->vao);
	glBindVertexArray(glBuff->vao);

	glGenBuffers(1, &glBuff->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, glBuff->vbo);

	if (createInfo->usage == RZ_BUFFER_USAGE_STATIC) {
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	} else if (createInfo->usage == RZ_BUFFER_USAGE_DYNAMIC) {
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return buffer;
}

void rzglUpdateBuffer(RZRenderContext* ctx, RZBuffer* buffer, void* data, size_t size) {

}

void rzglBindBuffer(RZRenderContext* ctx, RZBuffer* buffer) {
	
}

void rzglFreeBuffer(RZRenderContext* ctx, RZBuffer* buffer) {

}

void rzglLoadPFN(RZRenderContext* ctx) {
	ctx->init = rzglInit;
	ctx->createWindow = rzglCreateWindow;
	ctx->clear = rzglClear;
	ctx->setClearColor = rzglSetClearColor;
	ctx->swap = rzglSwap;

	ctx->allocBuffer = rzglAllocateBuffer;
	ctx->updateBuffer = rzglUpdateBuffer;
	ctx->bindBuffer = rzglBindBuffer;
	ctx->freeBuffer = rzglFreeBuffer;
}