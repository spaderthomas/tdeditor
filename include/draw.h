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
	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);
	glGetProgramiv(vs, GL_COMPILE_STATUS, &success);
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
	glShaderSource(fs, 1, &fs_source, NULL);
	glCompileShader(fs);
	glGetProgramiv(fs, GL_COMPILE_STATUS, &success);
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
	
	glGetProgramiv(*shader, GL_LINK_STATUS, &success);
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
	uint32 as_shader = *(uint32*)info;
	if (as_shader == basic_shader) {
		Basic_Shader_Info basic_info = info->basic;
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

// Text rendering
struct Character {
	uint32 texture;
	Vec2 size;
	Vec2 bearing;
	float advance;
};
typedef struct Character Character;
Character characters[128] = {0};

void character_add(FT_Face face, char c) {
	if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
		char* error_msg = malloc(64 * sizeof(char));
		sprintf(error_msg, "FreeType failed to load character: %c", c);
		TDNS_LOG(error_msg);
	}

	Character* character = characters + c;
	glGenTextures(1, &character->texture);
	glBindTexture(GL_TEXTURE_2D, character->texture);
	glTexImage2D(GL_TEXTURE_2D,
				 0,
				 GL_RED,
				 face->glyph->bitmap.width, face->glyph->bitmap.rows,
				 0,
				 GL_RED,
				 GL_UNSIGNED_BYTE,
				 face->glyph->bitmap.buffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	character->size.x = face->glyph->bitmap.width / g_viewport.x;
	character->size.y = face->glyph->bitmap.rows / g_viewport.y;
	character->bearing.x = face->glyph->bitmap_left  / g_viewport.x;
	character->bearing.y = face->glyph->bitmap_top  / g_viewport.y;
	character->advance = face->glyph->advance.x * 64  / g_viewport.x; // FT stores this in 1/64 pixel increments 
}

void FreeType_Init() {
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		TDNS_LOG("Error initializing FreeType");
	}
	
	FT_Face face;
	if (FT_New_Face(ft, "C:\\Windows\\Fonts\\arial.ttf", 0, &face)) {
		TDNS_LOG("Error loading font");
	}

	FT_Set_Pixel_Sizes(face, 0, 48);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	for (char c = 'a'; c <= 'z'; c++) {
		character_add(face, c);
	}
}

typedef struct Draw_Command {
	uint32 elem_count;
	Shader_Info shader_info;
} Draw_Command;

typedef struct Draw_List {
	Vertex* vertex_buffer;
	uint32* element_buffer;
	Draw_Command* command_buffer;
	uint32 last_texture;
} Draw_List;

void dl_render(Draw_List* draw_list) {
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
		Draw_Command* cmd = draw_list->command_buffer + idx;
		shader_setup(&cmd->shader_info);

		glDrawElements(GL_TRIANGLES, cmd->elem_count, GL_UNSIGNED_INT, (GLvoid*)(elements_drawn * sizeof(uint32)));

		elements_drawn += cmd->elem_count;
	}
}

void dl_reset(Draw_List* draw_list) {
	sb_clear(draw_list->command_buffer);
	sb_resetn(draw_list->vertex_buffer);
	sb_resetn(draw_list->element_buffer);
}

// Copies the elements pointed to into the draw list's vertex buffer
void dl_push_elements(Draw_List* draw_list, uint32* elements, uint32 count) {
	uint32 vert_idx = sb_count(draw_list->vertex_buffer) ?
		sb_count(draw_list->vertex_buffer) :
		0;
	fox_for(idx, count) {
		sb_push(draw_list->element_buffer, elements[idx] + vert_idx);
	}
}

// Copies the vertices pointed to into the draw list's vertex buffer
void dl_push_vertices(Draw_List* draw_list, Vertex* verts, uint32 count) {
	fox_for(idx, count) {
		sb_push(draw_list->vertex_buffer, verts[idx]);
	}
}

void dl_push_primitive(Draw_List* draw_list,
					   Vertex* vertices, uint32 count_verts,
					   uint32* elements, uint32 count_elems,
					   Draw_Command* cmd) {
	dl_push_elements(draw_list, elements, count_elems);
	dl_push_vertices(draw_list, vertices, count_verts);
	sb_push(draw_list->command_buffer, *cmd);	
}

void dl_push_text(Draw_List* draw_list, const char* text) {
	Vec2 point = { 0.f, 0.f };
	uint32 len = strlen(text);
	fox_for(idx, len) {
		Character* c = characters + idx;

		// Make a rectangle:
		//   Left: point.x + bearing.x
		//   Bottom: point.y - (height - bearing.y)
		//   Right: point.x + bearing.x + size.x
		//   Top: point.y + bearing.y
		float top = point.y + c->bearing.y;
		float bottom = point.y - (c->size.y - c->bearing.y);
		float left = point.x + c->bearing.x;
		float right = point.x + c->bearing.x + c->size.x;

		Vertex bottom_left = {
			left, bottom,
			1.f
		};
		
	}
}
