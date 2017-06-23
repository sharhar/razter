#include <razter/razter.h>
#include <stdio.h>

char* rzReadFileFromPath(char *filename, size_t* size) {
	char *buffer = NULL;
	size_t string_size, read_size;
	FILE *handler = fopen(filename, "rb");

	if (handler) {
		fseek(handler, 0, SEEK_END);
		string_size = ftell(handler);
		rewind(handler);

		buffer = (char*)malloc(sizeof(char) * (string_size + 1));

		read_size = fread(buffer, sizeof(char), string_size, handler);

		buffer[string_size] = '\0';

		if (string_size != read_size) {
			printf("Error occured while reading file!\nstring_size = %zu\nread_size = %zu\n\n", string_size, read_size);
			free(buffer);
			buffer = NULL;
		}

		*size = read_size;

		fclose(handler);
	}
	else {
		printf("Did not find file!\n");
	}

	return buffer;
}

RZContext* rzCreateContext(RZAPI type) {
	RZContext* ctx = malloc(sizeof(RZContext));

	if (type == RZ_API_VULKAN) {
		if (glfwVulkanSupported()) {
			rzvkLoadPFN(ctx);
		} else {
			return NULL;
		}
	}
#ifdef __APPLE__
	else if (type == RZ_API_METAL) {
		rzmtLoadPFN(ctx);
	}
#endif
	else {
		return NULL;
	}

	return ctx;
}
