#include <glad/glad.h>
#include <razter/razter.h>
#include <stdio.h>
#include <math.h>

typedef struct GLCTX {
	GLFWwindow* window;
} GLCTX;

typedef struct GLBuffer {
	GLuint vao;
	GLuint vbo;
	uint32_t count;
} GLBuffer;

typedef struct GLShader {
	GLuint vertShader;
	GLuint fragShader;
	GLuint shaderProgram;
} GLShader;

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

	glfwSwapInterval(1);
	
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
	
	//GLenum error = GL_NO_ERROR;
	//while((error = glGetError()) != GL_NO_ERROR) {
	//	printf("GLError = %d\n", error);
	//}
}

RZBuffer* rzglAllocateBuffer(RZRenderContext* ctx, RZBufferCreateInfo* createInfo, void* data, size_t size) {
	GLBuffer* buffer = malloc(sizeof(GLBuffer));

	buffer->count = createInfo->vertexAttribDesc->count;

	glGenVertexArrays(1, &buffer->vao);
	glBindVertexArray(buffer->vao);

	glGenBuffers(1, &buffer->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);

	if (createInfo->type == RZ_BUFFER_TYPE_STATIC) {
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	} else if (createInfo->type == RZ_BUFFER_TYPE_DYNAMIC) {
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}

	for (uint32_t i = 0; i < buffer->count;i++) {
		glVertexAttribPointer(i, 
			createInfo->vertexAttribDesc->sizes[i], 
			GL_FLOAT, GL_FALSE, 
			createInfo->vertexAttribDesc->stride,
			createInfo->vertexAttribDesc->offsets[i]);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	for (uint32_t i = 0; i < buffer->count; i++) {
		glEnableVertexAttribArray(i);
	}
	
	glBindVertexArray(0);

	return buffer;
}

void rzglUpdateBuffer(RZRenderContext* ctx, RZBuffer* buffer, void* data, size_t size) {
	//GLBuffer* glBuff = malloc(sizeof(GLBuffer));
}

void rzglBindBuffer(RZRenderContext* ctx, RZBuffer* buffer) {
	GLBuffer* glBuff = (GLBuffer*)buffer;
	
	glBindVertexArray(glBuff->vao);
}

void rzglFreeBuffer(RZRenderContext* ctx, RZBuffer* buffer) {

}

RZShader* rzglCreateShader(RZRenderContext* ctx, RZShaderCreateInfo* createInfo) {
	GLShader* shader = malloc(sizeof(GLShader));

	shader->vertShader = glCreateShader(GL_VERTEX_SHADER);
	shader->fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	if (createInfo->isPath != RZ_FALSE) {
		size_t vertSize;
		char* vertShader = rzReadFileFromPath(createInfo->vertData, &vertSize);

		size_t fragSize;
		char* fragShader = rzReadFileFromPath(createInfo->fragData, &fragSize);

		glShaderSource(shader->vertShader, 1, (const GLchar**)&vertShader, 0);
		glShaderSource(shader->fragShader, 1, (const GLchar**)&fragShader, 0);
	} else {
		glShaderSource(shader->vertShader, 1, (const GLchar**)&createInfo->vertData, 0);
		glShaderSource(shader->fragShader, 1, (const GLchar**)&createInfo->fragData, 0);
	}

	glCompileShader(shader->vertShader);

	GLint compiled = 0;
	glGetShaderiv(shader->vertShader, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(shader->vertShader, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar* message = (GLchar*)malloc(sizeof(GLchar)*maxLength);
		glGetShaderInfoLog(shader->vertShader, maxLength, &maxLength, message);

		printf("Vertex Shader failed to compile:\n%s\n", message);
		
		glDeleteShader(shader->vertShader);
		return NULL;
	}

	glCompileShader(shader->fragShader);

	glGetShaderiv(shader->fragShader, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(shader->fragShader, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar* message = (GLchar*)malloc(sizeof(GLchar)*maxLength);
		glGetShaderInfoLog(shader->fragShader, maxLength, &maxLength, message);

		printf("Fragment Shader failed to compile:\n%s\n", message);

		glDeleteShader(shader->fragShader);
		return NULL;
	}

	shader->shaderProgram = glCreateProgram();

	glAttachShader(shader->shaderProgram, shader->vertShader);
	glAttachShader(shader->shaderProgram, shader->fragShader);
	
	glLinkProgram(shader->shaderProgram);
	glValidateProgram(shader->shaderProgram);

	return shader;
}

void rzglBindShader(RZRenderContext* ctx, RZShader* shader) {
	GLShader* glShader = (GLShader*)shader;
	
	glUseProgram(glShader->shaderProgram);
	
}

void rzglDestroyShader(RZRenderContext* ctx, RZShader* shader) {

}

void rzglDraw(RZRenderContext* ctx, uint32_t firstVertex, uint32_t vertexCount) {
	glDrawArrays(GL_TRIANGLES, firstVertex, vertexCount);
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

	ctx->createShader = rzglCreateShader;
	ctx->bindShader = rzglBindShader;
	ctx->destroyShader = rzglDestroyShader;

	ctx->draw = rzglDraw;
}
