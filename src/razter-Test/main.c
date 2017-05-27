#include <glad/glad.h>
#include <razter/razter.h>

int main() {
	glfwInit();

	RZPlatform platform = RZ_PLATFORM_OPENGL;

	RZRenderContext* ctx = rzCreateRenderContext(platform);

	GLFWwindow* window = rzCreateWindow(ctx, 800, 600, "Razter Test");

	rzSetClearColor(ctx, 0.0f, 1.0f, 1.0f, 1.0f);

	float verts[] = {
		-0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		 0.0f,  0.5f, 0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f
	};

	size_t offsets[] = { 0, sizeof(float) * 2 };
	size_t stride = sizeof(float) * 5;
	uint32_t sizes[] = { 2, 3 };

	RZVertexAttributeDescription vertexAttribDesc;
	vertexAttribDesc.count = 2;
	vertexAttribDesc.stride = stride;
	vertexAttribDesc.offsets = offsets;
	vertexAttribDesc.sizes = sizes;

	RZBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.usage = RZ_BUFFER_USAGE_VERTEX;
	bufferCreateInfo.type = RZ_BUFFER_TYPE_STATIC;
	bufferCreateInfo.vertexAttribDesc = &vertexAttribDesc;

	RZBuffer* buffer = rzAllocateBuffer(ctx, &bufferCreateInfo, verts, sizeof(float) * 3 * 5);

	RZShaderCreateInfo shaderCreateInfo;
	shaderCreateInfo.isPath = RZ_TRUE;
	shaderCreateInfo.vertexAttribDesc = &vertexAttribDesc;
	shaderCreateInfo.vertData = "res/shader.vert";
	shaderCreateInfo.fragData = "res/shader.frag";
	shaderCreateInfo.vertSize = 0;
	shaderCreateInfo.fragSize = 0;

	RZShader* shader = rzCreateShader(ctx, &shaderCreateInfo);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		rzClear(ctx);

		rzBindBuffer(ctx, buffer);
		rzBindShader(ctx, shader);
		rzDraw(ctx, 0, 3);

		rzSwap(ctx);
	}

	glfwTerminate();
	return 0;
}