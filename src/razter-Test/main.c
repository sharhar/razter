#include <razter/razter.h>

int main() {
	glfwInit();

	RZRenderContext* ctx = rzCreateRenderContext(RZ_RC_DX);

	GLFWwindow* window = rzCreateWindow(ctx, 800, 600, "Razter Test");

	rzSetClearColor(ctx, 0.0f, 1.0f, 1.0f, 1.0f);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		rzClear(ctx);

		rzSwap(ctx);
	}

	glfwTerminate();
	return 0;
}