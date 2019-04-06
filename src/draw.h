uint32 basic_shader;
uint32 textured_shader;
uint32 text_shader;

// Use this to determine how far to move down for the next line
float g_max_char_height;

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

// Text rendering
struct Character {
	uint32 texture;
	Vec2 size;
	Vec2 bearing;
	float advance;
};
typedef struct Character Character;

// In raw pixel units; use these to recalculate when the viewport changes
Character px_char_infos[128] = {0};
// In screen units; use these when you're calculating vertices to draw! 
Character char_infos[128] = {0};

void character_add(FT_Face face, char c) {
	if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
		char* error_msg = malloc(64 * sizeof(char));
		sprintf(error_msg, "FreeType failed to load character: %c", c);
		TDNS_LOG(error_msg);
	}

	Character* px_info = px_char_infos + c;
	Character* screen_info = char_infos + c;
	glGenTextures(1, &px_info->texture);
	glBindTexture(GL_TEXTURE_2D, px_info->texture);
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

	px_info->size.x = face->glyph->bitmap.width;
	px_info->size.y = face->glyph->bitmap.rows;
	px_info->bearing.x = face->glyph->bitmap_left;
	px_info->bearing.y = face->glyph->bitmap_top;
	px_info->advance = (float)face->glyph->advance.x / 64; // FT stores this in 1/64 pixel increments

	screen_info->texture = px_info->texture;
	screen_info->size.x = px_info->size.x / (float)g_viewport.x;
	screen_info->size.y = px_info->size.y / (float)g_viewport.y;
	screen_info->bearing.x = px_info->bearing.x / (float)g_viewport.x;
	screen_info->bearing.y = px_info->bearing.y / (float)g_viewport.y;
	screen_info->advance = px_info->advance / (float)g_viewport.x;

	g_max_char_height = max(g_max_char_height, px_info->size.y);
}

void FreeType_Init() {
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		TDNS_LOG("Error initializing FreeType");
	}
	
	FT_Face face;
	char* font_path = td_strcat(get_conf("font_dir"), get_conf("font_default"));
	if (FT_New_Face(ft, font_path, 0, &face)) {
		TDNS_LOG("Error loading font");
	}

	FT_Set_Pixel_Sizes(face, 0, 48);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	for (char c = 'a'; c <= 'z'; c++) {
		character_add(face, c);
	}

	for (char c = 'A'; c <= 'Z'; c++) {
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

void dl_push_text(Draw_List* draw_list, char* text) {
	if (!text) return;
	
	Vec2 point = { -1.f, 1.f }; // Point always refers to top right
	uint32 len = strlen(text);
	fox_for(idx, len) {
		char c = text[idx];
		Character* char_info = &char_infos[c];
		
		// Make a rectangle:
		float top = point.y;
		float bottom = point.y - (char_info->size.y);
		float left = point.x + char_info->bearing.x;
		float right = point.x + char_info->bearing.x + char_info->size.x;

		Vertex bottom_left = {
		   {left, bottom},
		   {1.f, 1.f, 1.f},
		   {0.f, 1.f}
		};
		Vertex bottom_right = {
			{right, bottom},
			{1.f, 1.f, 1.f},
			{1.f, 1.f}
		};
		Vertex top_left = {
    		{left, top},
	    	{1.f, 1.f, 1.f},
    		{0.f, 0.f}
		};
		Vertex top_right = {
		    {right, top},
		    {1.f, 1.f, 1.f},
		    {1.f, 0.f}
		};

		Vertex verts[4] = {
		    top_right,
		    bottom_right,
		    bottom_left,
		    top_left
	    };
		
	    uint32 indices[6] = {
	    	0, 1, 3,
	    	1, 2, 3
    	};
		
		Draw_Command draw_cmd;
		draw_cmd.elem_count = 6;
		make_text_shader_info(shader_info);
		shader_info.text.texture = char_info->texture;
		draw_cmd.shader_info = shader_info;

		dl_push_primitive(draw_list, verts, 4, indices, 6, &draw_cmd);

		point.x += char_info->advance;

	}
}
