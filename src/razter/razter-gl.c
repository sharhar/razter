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

GLFWwindow* rzglCreateWindow(GLCTX* ctx, int width, int height, const char* title) {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	glfwMakeContextCurrent(window);
	
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwSwapInterval(1);
	
	ctx->window = window;
	
	return window;
}

void rzglClear(GLCTX* ctx) {
	glClear(GL_COLOR_BUFFER_BIT);
}

void rzglSetClearColor(GLCTX* ctx, float r, float g, float b, float a) {
	glClearColor(r, g, b, a);
}

void rzglSwap(GLCTX* ctx) {
	glfwSwapBuffers(ctx->window);
}

RZBuffer* rzglAllocateBuffer(GLCTX* ctx, RZBufferCreateInfo* createInfo, void* data, size_t size) {
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

void rzglUpdateBuffer(GLCTX* ctx, GLBuffer* buffer, void* data, size_t size) {
	
}

void rzglBindBuffer(GLCTX* ctx, GLBuffer* buffer) {
	glBindVertexArray(buffer->vao);
}

void rzglFreeBuffer(GLCTX* ctx, GLBuffer* buffer) {

}

RZShader* rzglCreateShader(GLCTX* ctx, RZShaderCreateInfo* createInfo) {
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

void rzglBindShader(GLCTX* ctx, GLShader* shader) {
	glUseProgram(shader->shaderProgram);
}

void rzglDestroyShader(GLCTX* ctx, GLShader* shader) {

}

void rzglDraw(GLCTX* ctx, uint32_t firstVertex, uint32_t vertexCount) {
	glDrawArrays(GL_TRIANGLES, firstVertex, vertexCount);
}

RZUniform* rzglCreateUniform(GLCTX* ctx, GLShader* shader) {
	GLUniform* uniform = malloc_c(sizeof(GLUniform));

	uniform->variableCount = shader->uniformCount;
	uniform->variables = malloc_c(sizeof(GLUniformVariable) * uniform->variableCount);

	for (uint32_t i = 0; i < uniform->variableCount; i++) {
		uniform->variables[i].data = NULL;
		uniform->variables[i].location = shader->locations[i];
		uniform->variables[i].type = shader->descriptors[i].type;
		uniform->variables[i].size = shader->descriptors[i].bufferSize;
	}

	return uniform;
}

void rzglBindUniform(GLCTX* ctx, GLShader* shader, GLUniform* uniform) {
	for (uint32_t i = 0; i < uniform->variableCount;i++) {
		RZUniformType type = uniform->variables[i].type;

		if (type == RZ_UNIFORM_TYPE_MATRIX_4) {
			glUniformMatrix4fv(uniform->variables[i].location, 1, GL_FALSE, uniform->variables[i].data);
		}
	}
}

void rzglUniformData(GLCTX* ctx, GLUniform* uniform, uint32_t index, void* data) {
	if (uniform->variables[index].data != NULL) {
		free_c(uniform->variables[index].data);
	}

	uniform->variables[index].data = malloc_c(uniform->variables[index].size);
	memcpy(uniform->variables[index].data, data, uniform->variables[index].size);
}

void rzglDestroyUniform(GLCTX* ctx, GLUniform* uniform) {

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
