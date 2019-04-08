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
#include "input.h"
#include "glfw_callbacks.h"

int main(int argc, char** argv) {
	load_config(&g_config);

	
	// GLFW init
	glfwSetErrorCallback(glfw_error_callback);

	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(1080, 1920, "tdeditor", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, glfw_resize_callback);
	glfwSetKeyCallback(window, glfw_key_callback);

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
	char* root_dir = get_conf("root_dir");

	char* basic_vs_path = td_strcat(root_dir, "assets/shaders/basic.vs");
	char* basic_fs_path = td_strcat(root_dir, "assets/shaders/basic.fs");
	shader_init(&basic_shader, basic_vs_path, basic_fs_path);

	char* text_vs_path = td_strcat(root_dir, "assets/shaders/text.vs");
	char* text_fs_path = td_strcat(root_dir, "assets/shaders/text.fs");
	shader_init(&text_shader, text_vs_path, text_fs_path);

	char* textured_vs_path = td_strcat(root_dir, "assets/shaders/textured.vs");
	char* textured_fs_path = td_strcat(root_dir, "assets/shaders/textured.fs");
	shader_init(&textured_shader, textured_vs_path, textured_fs_path);
	
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
	unsigned char* data = stbi_load(td_strcat(root_dir, "assets/container.jpg"), &width, &height, &count_channels, 0);
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
		{0.0f, 0.0f},
		{0.f, 0.f, 0.f},
		{1.f, 0.f}
	};
	Vertex bottom_right = {
		{.0f, -1.f},
		{0.f, 0.f, 0.f},
		{1.f, 1.f}
	};
	Vertex bottom_left = {
	    {-1.f, -1.f},
	    {0.f, 0.f, 0.f},
	    {0.f, 1.f}
	};
	Vertex top_left = {
	    {-1.f, 0.f},
	    {0.f, 0.f, 0.f},
	    {0.f, 0.f}
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

	tdstr input_buffer;
	tdstr_init(&input_buffer);
	float r = .5, g = .7, b = .9, a = 1;
	while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

		for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_GRAVE_ACCENT; key++) {
			if (was_pressed(key)) {
				char c = key;
				tdstr_push(&input_buffer, c);
			}
		}
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT);

		dl_push_text(&draw_list, input_buffer.buf);
		
		dl_render(&draw_list);
		dl_reset(&draw_list);
		
 		glfwSwapBuffers(window);
		reset_input();
	}
	
	printf("hahahahahaahahah!");
	return 1;
}
