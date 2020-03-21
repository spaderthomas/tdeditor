void editor_init() {
	state()->frame = 0;
	
	Buffer* fundamental = calloc(sizeof(Buffer), 1);
	buf_init(fundamental);
	fundamental->name = "fundamental";
	fundamental->active = true;
	fundamental->bottom = GL_BOTTOM + .1;
	activate_mode(&text_mode, fundamental);
	activate_mode(&fundamental_mode, fundamental);
	buf_add_info(fundamental);
	buf_add(fundamental);

	Buffer* minibuf = calloc(sizeof(Buffer), 1);
	buf_init(minibuf);
	minibuf->top = GL_BOTTOM + .1;
	activate_mode(&text_mode, minibuf);
	activate_mode(&fundamental_mode, minibuf);
	buf_add(minibuf);
	
	state()->draw_list = calloc(1, sizeof(DrawList));
}

// Probably should return the address of a static variable instead of allocating,
// but who cares?
EditorState* state() {
	static EditorState* state = NULL;

	if (!state) {
		state = calloc(1, sizeof(EditorState));
		
		// Setup the viewport to default to 1080p
		state->screen_info.viewport.x = 1920;
		state->screen_info.viewport.y = 1080;

		state->fonts = NULL;
		state->modes = NULL;
	}

	return state;
}

void buf_init(Buffer* buffer) {
	buffer->left = -1;
	buffer->right = 1;
	buffer->top = 1;
	buffer->bottom = -1;
	buffer->cursor_idx = 0;
	tdstr_init(&buffer->contents);
	buffer->active = false;
	buffer->visible = true;
	buffer->info = NULL;
	buffer->next = NULL;
}

void buf_copy(Buffer* dest, Buffer* src) {
	dest->left = src->left;
	dest->right = src->right;
	dest->top = src->top;
	dest->bottom = src->bottom;
	dest->cursor_idx = src->cursor_idx;
	dest->active = src->active;
	dest->visible = src->visible;
	dest->next = src->next;
	dest->name = src->name;

	tdstr_copy(&dest->contents, &src->contents);
}

void buf_draw(Buffer* buffer) {
	FontInfo* font = shget(state()->fonts, default_font);

	DrawCommand cmd;
	cmd.shader_info.basic.shader = basic_shader;
	cmd.mode = GL_LINES;
	if (buffer->info) {
		dl_push_rect_outline(
							 state()->draw_list,
							 buffer->top,
							 buffer->bottom + font->max_char_height,
							 buffer->left, buffer->right,
							 cmd);
		
		buf_draw(buffer->info);
	} else {
		dl_push_rect_outline(
							 state()->draw_list,
							 buffer->top,
							 buffer->bottom,
							 buffer->left, buffer->right,
							 cmd);
	}
	
	char* text = buffer->contents.buf;
	uint32 len = strlen(text);
	
	Vec2 point = { buffer->left, buffer->top - font->max_char_height };

	fox_for(idx, len) {
		if (idx == buffer->cursor_idx) {
			if (buffer->active) draw_cursor(font, point);
		}
	
		char c = text[idx];
		if (c == '\n') {
			point.x = buffer->left;
			point.y -= font->max_char_height;
			continue;
		}

		dl_push_character(state()->draw_list, c, point, font);
		advance_point(&point, c, font, buffer->left, buffer->right);
	}
	
	if (len == buffer->cursor_idx) {
		if (buffer->active) draw_cursor(font, point);
	}

}

Buffer* buf_last() {
	Buffer* buffer = state()->buf_first;
	while (buffer && buffer->next) {
		buffer = buffer->next;
	}
	return buffer;
}

Buffer* buf_active() {
	Buffer* buffer = state()->buf_first;
	while (buffer && !buffer->active) {
		buffer = buffer->next;
	}
	return buffer;
}

void buf_add(Buffer* buffer) {
	Buffer* last = buf_last();
	if (!last) {
		state()->buf_first = buffer;
		return;
	}
	
	last->next = buffer;
}

Buffer* buf_first() {
	return state()->buf_first;
}

void buf_add_info(Buffer* buffer) {
	// @hack
	FontInfo* font = shget(state()->fonts, default_font);

	Buffer* info = calloc(sizeof(Buffer), 1);
	buf_init(info);
	info->top = buffer->bottom + font->max_char_height;
	info->bottom = buffer->bottom;
	info->left = buffer->left;
	info->right = buffer->right;
	info->contents = tdstr_from_c(buffer->name);

	buffer->info = info;
}

// Push a character to the buffer if it was pressed
// GLFW key codes correspond to ASCII codes, so if shift isn't down, subtract 32 to get lower case
void maybe_insert_char(Buffer* buffer, int key) {
	if (was_pressed(key)) {
		if (is_shift_down()) {
			tdstr_insert(&buffer->contents, key, buffer->cursor_idx);
			buffer->cursor_idx++;
		} else {
			tdstr_insert(&buffer->contents, key + 32, buffer->cursor_idx);
			buffer->cursor_idx++;
		}
	}
}

// Push a symbol or its 'shift' version to the buffer if it was pressed
void maybe_insert_symbol(Buffer* buffer, int key) {
	if (was_pressed(key)) {
		if (is_shift_down()) {
			tdstr_insert(&buffer->contents, shift_map[key], buffer->cursor_idx);
			buffer->cursor_idx++;
		} else {
			tdstr_insert(&buffer->contents, key, buffer->cursor_idx);
			buffer->cursor_idx++;
		}
	}
}

void handle_input(Buffer* buffer) {
	// Try to run any commands in the modes of this buffer
	fox_for(i, sb_count(buffer->modes)) {
		Mode* mode = buffer->modes[i];
		fox_for(j, sb_count(mode->commands)) {
			Command* cmd = mode->commands[j];
			if (is_mod_key_down(cmd->mod) && was_pressed(cmd->key)) {
				cmd->func(buffer);
			}
			
		}
	}

	// If we're not using a command key, push input characters to the buffer
	if (!is_ctrl_down() && !is_meta_down()) {
		for(int key = GLFW_KEY_SPACE; key <= GLFW_KEY_EQUAL; key++) {
			maybe_insert_symbol(buffer, key);
		}

		for(int key = GLFW_KEY_A; key <= GLFW_KEY_Z; key++) {
			maybe_insert_char(buffer, key);
		}

		for(int key = GLFW_KEY_LEFT_BRACKET; key <= GLFW_KEY_GRAVE_ACCENT; key++) {
			maybe_insert_symbol(buffer, key);
		}
	}
}

void update() {
	Buffer* buffer = buf_active();
	
	fancy_assert(buffer) {
		uint16 log_flags = STDERR_FILENO;
		TD_LOG("There was no active buffer while handling input.\n");
		TD_LOG2("There was no active buffer while handling input.\n", log_flags); // @remove
	}
	
	// Move the input into the text buffer
	handle_input(buffer);

	buffer = buf_first();
	while (buffer) {
		if (buffer->visible) buf_draw(buffer);
		buffer = buffer->next;
	}
	
    state()->frame++;
}

void next_frame() {
	dl_reset(state()->draw_list);
}

void render() {
	dl_render(state()->draw_list);
}

void draw_cursor(FontInfo* font, Vec2 point) {
	Vec2 a = { point.x, point.y };
	Vec2 b = { point.x, point.y + font->max_char_height };
	DrawCommand cmd;
	cmd.shader_info.basic.shader = basic_shader;
	dl_push_line(state()->draw_list, a, b, cmd);
}


void register_mode(EditorState* state, Mode* mode) {
	sb_push(state->modes, mode);
}

void register_cmd(Mode* mode, Command* cmd) {
	// GLFW reads all keys as upper, so convert them once on initialization
	if (is_lower(cmd->key)) {
		cmd->key = char_upper(cmd->key);
	}

	// @hack Really long mode name could cause a memory error
	char* message = calloc(sizeof(char), 128);
	sprintf(message, "[%s-mode message] ", mode->name);
	print_color(CONSOLE_BLUE, message);
	printf("Command registered with mod = %d, key = %d\n", cmd->mod, cmd->key);
	sb_push(mode->commands, cmd);
}

void activate_mode(Mode* mode, Buffer* buffer) {
	char* message = calloc(sizeof(char), 128);
	sprintf(message, "[%s-mode message] ", mode->name);
	print_color(CONSOLE_BLUE, message);
	printf("%s-mode activated in [%s]\n", mode->name, buffer->name);
	sb_push(buffer->modes, mode);
}

// Buffer API!
void advance_point(Vec2* point, char c, FontInfo* font, float lbound, float rbound) {
	FontChar* char_info = &font->screen_infos[c];
	point->x += char_info->advance;
	if (point->x > rbound) {
		point->x = lbound;
		point->y -= font->max_char_height;
	}

}
	

int td_buf_len(Buffer* buffer) {
	return buffer->contents.len;
}

void td_delete_char_back(tdstr* buf, int i) {
	tdstr_remove(buf, i);
}

void td_new_line(tdstr* buf, int i) {
	tdstr_insert(buf, '\n', i);
}
