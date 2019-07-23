static Mode text_mode;

void do_begin_line(EditorState* state) {
	printf("do_begin_line\n");
	state->cursor_idx = 0;
}

void do_back_char(EditorState* state) {
	printf("do_back_char\n");
	state->cursor_idx = tdmax(state->cursor_idx - 1, 0);
}

void do_forward_char(EditorState* state) {
	printf("do_forward_char\n");
	state->cursor_idx = tdmin(state->cursor_idx + 1, td_buf_len(state));
}

void do_delete_char_back(EditorState* state) {
	printf("do_delete_char_back\n");
	td_delete_char_back(&state->contents, state->cursor_idx - 1);
	state->cursor_idx = tdmax(state->cursor_idx - 1, 0);
}

void do_new_line(EditorState* state) {
	printf("do_new_line\n");
	td_new_line(&state->contents, state->cursor_idx);
	state->cursor_idx = tdmin(state->cursor_idx + 1, td_buf_len(state));
}

static Command text_cmds[] = {
	CMD('a', Mod_CONTROL, do_begin_line)
	CMD('j', Mod_CONTROL, do_back_char)
	CMD('l', Mod_CONTROL, do_forward_char)
	CMD(GLFW_KEY_BACKSPACE, Mod_NONE, do_delete_char_back)
	CMD(GLFW_KEY_ENTER, Mod_NONE, do_new_line)
	CMD_END()
};

void text_mode_init() {
	text_mode.name = "text";

	// Loop through commands until we find CMD_END, registering each command
	Command* cmd = text_cmds;
	while(*((int*)cmd)) {
		register_cmd(&text_mode, cmd);
		cmd++;
	}

	EditorState* state = get_editor_state();
	register_mode(state, &text_mode);
}
