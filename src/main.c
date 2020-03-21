#include <stdio.h>
#include <stdint.h>

#ifndef _WIN32
#include "unix.h"
#else
#include "win32.h"
#endif

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "stretchy_buffer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define TD_BUFFER_RAW_STRING

#include "machine.h"
#include "utils.h"
#include "font.h"
#include "editor.h"
#include "modes.h"
#include "draw.h"
#include "input.h"
#include "glfw_callbacks.h"

#include "editor_impl.h"
#include "font_impl.h"
#include "modes_impl.h"

int main(int argc, char** argv) {
	load_config();
	EditorState* ctx = state();
	
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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallbackKHR(gl_debug_callback, NULL);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	}
	
	// Set up the shaders
	char* basic_vs_path = td_strcat(project_root, "assets/shaders/basic.vs");
	char* basic_fs_path = td_strcat(project_root, "assets/shaders/basic.fs");
	shader_init(&basic_shader, basic_vs_path, basic_fs_path);

	char* text_vs_path = td_strcat(project_root, "assets/shaders/text.vs");
	char* text_fs_path = td_strcat(project_root, "assets/shaders/text.fs");
	shader_init(&text_shader, text_vs_path, text_fs_path);

	char* textured_vs_path = td_strcat(project_root, "assets/shaders/textured.vs");
	char* textured_fs_path = td_strcat(project_root, "assets/shaders/textured.fs");
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
	unsigned char* data = stbi_load(td_strcat(project_root, "assets/container.jpg"), &width, &height, &count_channels, 0);
	if (!data) {
		TD_LOG("Could not load image");
		exit(0);
	}
	
	glGenTextures(1, &g_texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	glGenerateMipmap(GL_TEXTURE_2D);

	// Set up our global data structures
	load_default_font(ctx);
	fill_shift_map();

	glfwSetWindowSize(window, 800, 600);

	tdstr input_buffer;
	tdstr_init(&input_buffer);
	float r = .5, g = .7, b = .9, a = 1;

	float seconds_per_update = 1.f / 60.f;

	text_mode_init();
	fundamental_mode_init();

	/*
	Buffer* other_buffer = calloc(1, sizeof(Buffer));
	buf_init(other_buffer);
	other_buffer->name = "test";
	other_buffer->left = -1;
	other_buffer->right = 0;
	other_buffer->top = 1;
	other_buffer->bottom = -1;
	activate_mode(&text_mode, other_buffer);
	activate_mode(&fundamental_mode, other_buffer);
	add_buffer(other_buffer);
	*/
	
	while (!glfwWindowShouldClose(window)) {
		double frame_start_time = glfwGetTime();

        glfwPollEvents();

		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT);

		update();
		render();
		next_frame();
		
 		glfwSwapBuffers(window);
		reset_input();

		while (glfwGetTime() - frame_start_time < seconds_per_update) {}
	}
	
	printf("hahahahahaahahah!");
	return 1;
}
