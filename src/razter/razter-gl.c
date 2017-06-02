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

	RZUniformDescriptor* descriptors;
	GLuint* locations;
	uint32_t uniformCount;
} GLShader;

typedef struct GLUniformVariable {
	GLuint location;
	void* data;
	size_t size;
	RZUniformType type;
} GLUniformVariable;

typedef struct GLUniform {
	GLUniformVariable* variables;
	uint32_t variableCount;
} GLUniform;

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
	
}

void rzglBindBuffer(RZRenderContext* ctx, RZBuffer* buffer) {
	GLBuffer* glBuff = (GLBuffer*)buffer;
	
	glBindVertexArray(glBuff->vao);
}

void rzglFreeBuffer(RZRenderContext* ctx, RZBuffer* buffer) {

}

RZShader* rzglCreateShader(RZRenderContext* ctx, RZShaderCreateInfo* createInfo) {
	GLShader* shader = malloc(sizeof(GLShader));

	shader->uniformCount = createInfo->descriptorCount;
	shader->descriptors = malloc_c(sizeof(RZUniformDescriptor) * shader->uniformCount);
	memcpy(shader->descriptors, createInfo->descriptors, sizeof(RZUniformDescriptor) * shader->uniformCount);

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

	glUseProgram(shader->shaderProgram);

	shader->locations = malloc_c(sizeof(GLuint) * shader->uniformCount);

	for (uint32_t i = 0; i < shader->uniformCount;i++) {
		shader->locations[i] = glGetUniformLocation(shader->shaderProgram, createInfo->descriptors[i].name);
	}

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

RZUniform* rzglCreateUniform(RZRenderContext* ctx, RZShader* shader) {
	GLShader* glShader = (GLShader*)shader;
	GLUniform* uniform = malloc_c(sizeof(GLUniform));

	uniform->variableCount = glShader->uniformCount;
	uniform->variables = malloc_c(sizeof(GLUniformVariable) * uniform->variableCount);

	for (uint32_t i = 0; i < uniform->variableCount; i++) {
		uniform->variables[i].data = NULL;
		uniform->variables[i].location = glShader->locations[i];
		uniform->variables[i].type = glShader->descriptors[i].type;
		uniform->variables[i].size = glShader->descriptors[i].bufferSize;
	}

	return uniform;
}

void rzglBindUniform(RZRenderContext* ctx, RZShader* shader, RZUniform* uniform) {
	GLUniform* glUniform = (GLUniform*)uniform;

	for (uint32_t i = 0; i < glUniform->variableCount;i++) {
		RZUniformType type = glUniform->variables[i].type;

		if (type == RZ_UNIFORM_TYPE_MATRIX_4) {
			glUniformMatrix4fv(glUniform->variables[i].location, 1, GL_FALSE, glUniform->variables[i].data);
		}
	}
}

void rzglUniformData(RZRenderContext* ctx, RZUniform* uniform, uint32_t index, void* data) {
	GLUniform* glUniform = (GLUniform*)uniform;

	if (glUniform->variables[index].data != NULL) {
		free_c(glUniform->variables[index].data);
	}

	glUniform->variables[index].data = malloc_c(glUniform->variables[index].size);
	memcpy(glUniform->variables[index].data, data, glUniform->variables[index].size);
}

void rzglDestroyUniform(RZRenderContext* ctx, RZUniform* uniform) {

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

	ctx->createUniform = rzglCreateUniform;
	ctx->bindUniform = rzglBindUniform;
	ctx->uniformData = rzglUniformData;
	ctx->destroyUniform = rzglDestroyUniform;
}
