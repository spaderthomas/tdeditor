void editor_init(EditorState* state) {
	state->frame = 0;
	
	state->pane = calloc(sizeof(Pane), 1);
	pane_init(state->pane);
	state->pane->active = true;
	
	// Setup the viewport to default to 1080p
	state->screen_info.viewport.x = 1920;
	state->screen_info.viewport.y = 1080;
	
	state->fonts = NULL;
	
	state->modes = NULL;
	
	state->draw_list = calloc(1, sizeof(DrawList));
}

// Probably should return the address of a static variable instead of allocating,
// but who cares?
EditorState* state() {
	static EditorState* state = NULL;
	if (!state) {
		state = calloc(1, sizeof(EditorState));
		editor_init(state);
	}

	return state;
}

void pane_init(Pane* pane) {
	pane->left = 0;
	pane->right = 0;
	pane->top = 0;
	pane->bottom = 0;
	pane->cursor_idx = 0;
	tdstr_init(&pane->contents);
	pane->active = false;
	pane->next = NULL;
}

// Push a character to the buffer if it was pressed
// GLFW key codes correspond to ASCII codes, so if shift isn't down, subtract 32 to get lower case
void maybe_insert_char(Pane* pane, int key) {
	if (was_pressed(key)) {
		if (is_shift_down()) {
			tdstr_insert(&pane->contents, key, pane->cursor_idx);
			pane->cursor_idx++;
		} else {
			tdstr_insert(&pane->contents, key + 32, pane->cursor_idx);
			pane->cursor_idx++;
		}
	}
}

// Push a symbol or its 'shift' version to the buffer if it was pressed
void maybe_insert_symbol(Pane* pane, int key) {
	if (was_pressed(key)) {
		if (is_shift_down()) {
			tdstr_insert(&pane->contents, shift_map[key], pane->cursor_idx);
			pane->cursor_idx++;
		} else {
			tdstr_insert(&pane->contents, key, pane->cursor_idx);
			pane->cursor_idx++;
		}
	}
}

void handle_input(Pane* pane) {
	// Try to run any commands in the modes of this pane
	fox_for(i, sb_count(pane->modes)) {
		Mode* mode = pane->modes[i];
		fox_for(j, sb_count(mode->commands)) {
			Command* cmd = mode->commands[j];
			if (is_mod_key_down(cmd->mod) && was_pressed(cmd->key)) {
				cmd->func(pane);
			}
			
		}
	}

	// If we're not using a command key, push input characters to the buffer
	if (!is_ctrl_down() && !is_meta_down()) {
		for(int key = GLFW_KEY_SPACE; key <= GLFW_KEY_EQUAL; key++) {
			maybe_insert_symbol(pane, key);
		}

		for(int key = GLFW_KEY_A; key <= GLFW_KEY_Z; key++) {
			maybe_insert_char(pane, key);
		}

		for(int key = GLFW_KEY_LEFT_BRACKET; key <= GLFW_KEY_GRAVE_ACCENT; key++) {
			maybe_insert_symbol(pane, key);
		}
	}
}

void update() {
	Pane* pane = state()->pane;
	while(pane && !pane->active) pane = pane->next;
	
	fancy_assert(pane) {
		uint16 log_flags = STDERR_FILENO;
		TD_LOG("There was no active pane while handling input.\n");
		TD_LOG2("There was no active pane while handling input.\n", log_flags); // @remove
	}

	// Move the cursor
	if (was_pressed(GLFW_KEY_RIGHT)) pane->cursor_idx++;
	if (was_pressed(GLFW_KEY_LEFT)) pane->cursor_idx--;

	// Move the input into the text buffer
	handle_input(pane);

	// Emit commands to draw the text + cursor
	draw_text(pane);
	
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
	dl_push_line(state()->draw_list, a, b, &cmd);
}

void draw_text(Pane* pane) {
	FontInfo* font = shget(state()->fonts, get_conf("font_default"));
	char* text = pane->contents.buf;
	uint32 len = strlen(text);
	
	Vec2 point = { -1.f, 1.f - font->max_char_height };

	if (!len) {
		draw_cursor(font, point);
		return;
	}
	
	fox_for(idx, len) {
		if (idx == pane->cursor_idx) {
			draw_cursor(font, point);
		}
		
		char c = text[idx];
		if (c == '\n') {
			point.x = -1;
			point.y -= font->max_char_height;
		}
		
		FontChar* char_info = &font->screen_infos[c];

		// Make a rectangle:
		float top = point.y + char_info->bearing.y;
		float bottom = point.y - char_info->size.y + char_info->bearing.y;
		float left = point.x + char_info->bearing.x;
		float right = point.x + char_info->bearing.x + char_info->size.x;

		DrawCommand cmd;
		cmd.shader_info.text.shader = text_shader;
		cmd.shader_info.text.texture = char_info->texture;
		dl_push_rect(state()->draw_list, top, bottom, left, right, &cmd);

		point.x += char_info->advance;
		if (point.x > 1) {
			point.x = -1;
			point.y -= font->max_char_height;
		}
	}

	if (len == pane->cursor_idx) {
		draw_cursor(font, point);
	}
}

void register_mode(EditorState* state, Mode* mode) {
	sb_push(state->modes, mode);
}

void register_cmd(Mode* mode, Command* cmd) {
	// GLFW reads all keys as upper, so convert them once on initialization
	if (is_lower(cmd->key)) {
		cmd->key = char_upper(cmd->key);
	}

	
	printf("%s-mode message: Command registered with mod = %d, key = %d\n", mode->name, cmd->mod, cmd->key);
	sb_push(mode->commands, cmd);
}

void activate_mode(Mode* mode, Pane* pane) {
	sb_push(pane->modes, mode);
}

// Buffer API!
int td_buf_len(Pane* pane) {
	return pane->contents.len;
}

void td_delete_char_back(tdstr* buf, int i) {
	tdstr_remove(buf, i);
}

void td_new_line(tdstr* buf, int i) {
	tdstr_insert(buf, '\n', i);
}
