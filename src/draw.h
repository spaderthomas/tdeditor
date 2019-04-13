uint32 basic_shader;
uint32 textured_shader;
uint32 text_shader;

void shader_init(uint32* shader, const char* vs_path, const char* fs_path) {
	// Grab the source code for the VS
	FILE* vs_file = fopen(vs_path, "rb");
	if (!vs_file) {
		TDNS_LOG("Error opening vertex shader. Shader was:");
		TDNS_LOG(vs_path);
	}
	char* vs_source = file_contents(vs_file);
	
	bool success;

	uint32 vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, (const GLchar* const*)&vs_source, NULL);
	glCompileShader(vs);
	glGetProgramiv(vs, GL_COMPILE_STATUS, (int*)&success);
	if (!success) {
		char* what = malloc(512 * sizeof(char));
		char* header = "Error compiling vertex shader!\n";
		strcpy(what, header);
		glGetShaderInfoLog(vs, 512, NULL, what + strlen(header));
		TDNS_LOG(what);
	}
	
	// Grab the source code for the FS
	FILE* fs_file = fopen(fs_path, "rb");
	if (!fs_file) {
		TDNS_LOG("Error opening fragment shader. Shader was:");
		TDNS_LOG(fs_path);
	}
	char* fs_source = file_contents(fs_file);
	
	uint32 fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, (const GLchar* const*)&fs_source, NULL);
	glCompileShader(fs);
	glGetProgramiv(fs, GL_COMPILE_STATUS, (int*)&success);
	if (!success) {
		char* what = malloc(512 * sizeof(char));
		char* header = "Error compiling fragment shader!\n";
		strcpy(what, header);
		glGetShaderInfoLog(fs, 512, NULL, what + strlen(header));
		TDNS_LOG(what);
	}
	
	*shader = glCreateProgram();
	glAttachShader(*shader, vs);
	glAttachShader(*shader, fs);
	glLinkProgram(*shader);
	
	glGetProgramiv(*shader, GL_LINK_STATUS, (int*)&success);
	if (!success) {
		char* what = malloc(512 * sizeof(char));
		char* header = "Error linking shader program!\n";
		strcpy(what, header);
		glGetShaderInfoLog(fs, 512, NULL, what + strlen(header));
		TDNS_LOG(what);
	}
}

// All of these structs use the program id as a tag 
struct Basic_Shader_Info {
	uint32 shader;
};
typedef struct Basic_Shader_Info Basic_Shader_Info;
#define make_basic_shader_info(var) Shader_Info var; var.basic.shader = basic_shader

struct Textured_Shader_Info {
	uint32 shader;
	uint32 texture;
};
typedef struct Textured_Shader_Info Textured_Shader_Info;
#define make_textured_shader_info(var) Shader_Info var; var.textured.shader = textured_shader

struct Text_Shader_Info {
	uint32 shader;
	uint32 texture;
};
typedef struct Text_Shader_Info Text_Shader_Info;
#define make_text_shader_info(var) Shader_Info var; var.text.shader = text_shader

union Shader_Info {
	Basic_Shader_Info basic;
	Textured_Shader_Info textured;
	Text_Shader_Info text;
};
typedef union Shader_Info Shader_Info;

void shader_setup(Shader_Info* info) {
	// The first element of each Shader_Info kind has an integer identifier
	uint32 as_shader = *(uint32*)info;
	
	if (as_shader == basic_shader) {
		glUseProgram(basic_shader);
	}
	else if (as_shader == textured_shader) {
		Textured_Shader_Info textured_info = info->textured;
		glUseProgram(textured_shader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textured_info.texture);
	}
	else if (as_shader == text_shader) {
		Text_Shader_Info text_info = info->text;
		glUseProgram(text_shader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, text_info.texture);
	}
}

typedef struct DrawCommand {
	uint32 count_elems;
	uint32 count_verts;
	Shader_Info shader_info;
	GLenum mode;
} DrawCommand;

// All of the buffers are stretchy_buffers 
typedef struct DrawList {
	Vertex* vertex_buffer;
	uint32* element_buffer;
	DrawCommand* command_buffer;
	uint32 last_texture;
} DrawList;

void dl_render(DrawList* draw_list) {
	glBindVertexArray(g_VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	glBufferData(GL_ARRAY_BUFFER,
				 sizeof(Vertex) * sb_count(draw_list->vertex_buffer),
				 draw_list->vertex_buffer,
				 GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 sizeof(uint32) * sb_count(draw_list->element_buffer),
				 draw_list->element_buffer,
				 GL_STATIC_DRAW);

	uint64 elements_drawn = 0; // 64 bits because must cast to a GLvoid*
	draw_list->last_texture = 0;
	fox_for(idx, sb_count(draw_list->command_buffer)) {
		DrawCommand* cmd = draw_list->command_buffer + idx;
		shader_setup(&cmd->shader_info);

		glDrawElements(cmd->mode, cmd->count_elems, GL_UNSIGNED_INT, (GLvoid*)(elements_drawn * sizeof(uint32)));

		elements_drawn += cmd->count_elems;
	}
}

void dl_reset(DrawList* draw_list) {
	sb_clear(draw_list->command_buffer);
	sb_resetn(draw_list->vertex_buffer);
	sb_resetn(draw_list->element_buffer);
}

// Copies the elements pointed to into the draw list's vertex buffer
void dl_push_elements(DrawList* draw_list, uint32* elements, uint32 count) {
	uint32 vert_idx = sb_count(draw_list->vertex_buffer) ?
		sb_count(draw_list->vertex_buffer) :
		0;
	fox_for(idx, count) {
		sb_push(draw_list->element_buffer, elements[idx] + vert_idx);
	}
}

// Copies the vertices pointed to into the draw list's vertex buffer
void dl_push_vertices(DrawList* draw_list, Vertex* verts, uint32 count) {
	fox_for(idx, count) {
		sb_push(draw_list->vertex_buffer, verts[idx]);
	}
}

// Base level function. Only provided so you never forget that you must push
// elements first to get the proper offset for vertices!
void dl_push_primitive(DrawList* draw_list,
					   Vertex* vertices, uint32* elements,
					   DrawCommand* cmd) {
	dl_push_elements(draw_list, elements, cmd->count_elems);
	dl_push_vertices(draw_list, vertices, cmd->count_verts);
	sb_push(draw_list->command_buffer, *cmd);
}

