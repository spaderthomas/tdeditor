#include <stdio.h>
#include <stdint.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "stretchy_buffer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "data_types.h"
#include "utils.h"
#include "draw.h"

int main(int argc, char** argv) {
	// GLFW init
	glfwSetErrorCallback(glfw_error_callback);

	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(1080, 1920, "tdeditor", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, glfw_resize_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		return -1;
	}    

	glViewport(0, 0, 800, 600);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		//glDebugMessageCallbackKHR(gl_debug_callback, NULL);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	}
	
	// Set up the shaders
	shader_init(&basic_shader, "C:/Programming/tdeditor/assets/shaders/basic.vs", "C:/Programming/tdeditor/assets/shaders/basic.fs");
	shader_init(&text_shader, "C:/Programming/tdeditor/assets/shaders/text.vs", "C:/Programming/tdeditor/assets/shaders/text.fs");
	shader_init(&textured_shader, "C:/Programming/tdeditor/assets/shaders/textured.vs", "C:/Programming/tdeditor/assets/shaders/textured.fs");

	// GL object init
	glGenVertexArrays(1, &g_VAO);
	glGenBuffers(1, &g_VBO);
	glGenBuffers(1, &EBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	
	glBindVertexArray(g_VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(5 * sizeof(float)));
	
	int32 width, height, count_channels;
	unsigned char* data = stbi_load("C:/Programming/tdeditor/assets/container.jpg", &width, &height, &count_channels, 0);
	if (!data) {
		TDNS_LOG("Could not load image");
		exit(0);
	}
	
	glGenTextures(1, &g_texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	glGenerateMipmap(GL_TEXTURE_2D);

	// Some test vertices
	Vertex top_right = {
		0.0f, 0.0f,
		0.f, 0.f, 0.f,
		1.f, 0.f
	};
	Vertex bottom_right = {
		.0f, -1.f,
		0.f, 0.f, 0.f,
		1.f, 1.f
	};
	Vertex bottom_left = {
		-1.f, -1.f,
		0.f, 0.f, 0.f,
		0.f, 1.f
	};
	Vertex top_left = {
		-1.f, 0.f,
		0.f, 0.f, 0.f,
		0.f, 0.f
	};

	Vertex test_vertices[4] = {
		top_right,
		bottom_right,
		bottom_left,
		top_left
	};

	uint32 square_indices[6] = {
		0, 1, 3,
		1, 2, 3
	};

	uint32 ur_triangle_indices[3] = {
		0, 1, 3,
	};

	uint32 ll_triangle_indices[3] = {
		1, 2, 3,
	};

	Draw_List draw_list = {0};
	FreeType_Init();

	float r = .5, g = .7, b = .9, a = 1;
	while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT);

		Draw_Command draw_cmd;
		draw_cmd.elem_count = 6;
		make_text_shader_info(shader_info);
		shader_info.text.texture = characters['y'].texture;
		draw_cmd.shader_info = shader_info;

		dl_push_primitive(&draw_list, test_vertices, 4, square_indices, 6, &draw_cmd);
			
		dl_render(&draw_list);
		dl_reset(&draw_list);
		
 		glfwSwapBuffers(window);
	}
	
	printf("hahahahahaahahah!");
	return 1;
}
