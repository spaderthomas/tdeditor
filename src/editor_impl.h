EditorState* get_editor_state() {
	static EditorState* state = NULL;
	if (!state) {
		state = calloc(1, sizeof(EditorState));
		state->frame = 0;
		state->cursor_idx = 0;
		
		tdstr_init(&state->contents);

		state->screen_info.viewport.x = 1920;
		state->screen_info.viewport.y = 1080;

		state->fonts = NULL;

		state->modes = NULL;
		
		state->draw_list = calloc(1, sizeof(DrawList));
	}

	return state;
}

// Push a character to the buffer if it was pressed
// GLFW key codes correspond to ASCII codes, so if shift isn't down, subtract 32 to get lower case
void maybe_insert_char(tdstr* buffer, int key) {
	EditorState* state = get_editor_state();
	
	if (was_pressed(key)) {
		if (is_shift_down()) {
			tdstr_insert(buffer, key, state->cursor_idx);
			state->cursor_idx++;
		} else {
			tdstr_insert(buffer, key + 32, state->cursor_idx);
			state->cursor_idx++;
		}
	}
}

// Push a symbol or its 'shift' version to the buffer if it was pressed
void maybe_insert_symbol(tdstr* buffer, int key) {
	EditorState* state = get_editor_state();

	if (was_pressed(key)) {
		if (is_shift_down()) {
			tdstr_insert(buffer, shift_map[key], state->cursor_idx);
			state->cursor_idx++;
		} else {
			tdstr_insert(buffer, key, state->cursor_idx);
			state->cursor_idx++;
		}
	}
}

// Applies modifiers and then sends the characters to the display buffer
void handle_input(tdstr* buffer) {
	EditorState* state = get_editor_state();

	// Try to run any commands in any active modes
	fox_for(i, sb_count(state->modes)) {
		Mode* mode = state->modes[i];
		fox_for(j, sb_count(mode->commands)) {
			Command* cmd = mode->commands[j];
			if (is_mod_key_down(cmd->mod) && was_pressed(cmd->key)) {
				cmd->func(state);
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


	if (was_pressed(GLFW_KEY_ENTER)) tdstr_push(buffer, '\n');
	if (was_pressed(GLFW_KEY_BACKSPACE)) tdstr_pop(buffer);
}

void update() {
	EditorState* ctx = get_editor_state();
	
	// Move the cursor
	if (was_pressed(GLFW_KEY_RIGHT)) ctx->cursor_idx++;
	if (was_pressed(GLFW_KEY_LEFT)) ctx->cursor_idx--;

	// Move the input into the text buffer
	handle_input(&ctx->contents);

	// Emit commands to draw the text + cursor
	draw_text(ctx);
	
    ctx->frame++;
}

void next_frame() {
	EditorState* ctx = get_editor_state();
	dl_reset(ctx->draw_list);
}

void render() {
	EditorState* ctx = get_editor_state();
	dl_render(ctx->draw_list);
}

void draw_text(EditorState* state) {
	FontInfo* font = shget(state->fonts, get_conf("font_default"));
	char* text = state->contents.buf;
	uint32 len = strlen(text);
	
	Vec2 point = { -1.f, 1.f - font->max_char_height };
	fox_for(idx, len) {
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
		dl_push_rect(state->draw_list, top, bottom, left, right, &cmd);

		point.x += char_info->advance;
		if (point.x > 1) {
			point.x = -1;
			point.y -= font->max_char_height;
		}

		if (idx == state->cursor_idx) {
			Vec2 a = { point.x, point.y };
			Vec2 b = { point.x, point.y + font->max_char_height };
			DrawCommand cmd;
			cmd.shader_info.basic.shader = basic_shader;
			dl_push_line(state->draw_list, a, b, &cmd);
		}
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

	printf("%s-mode message: Command registered with mod = %d, key = %c\n", mode->name, cmd->mod, cmd->key);
	sb_push(mode->commands, cmd);
}
