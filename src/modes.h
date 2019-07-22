static Mode text_mode;

void do_text_begin_line(EditorState* state) {
	state->cursor_idx = 0;
	printf("do_text_begin_line\n");
}

void do_back_char(EditorState* state) {
	state->cursor_idx--;
	printf("do_back_char\n");
}

void do_forward_char(EditorState* state) {
	state->cursor_idx++;
	printf("do_forward_char\n");
}

static Command text_cmds[] = {
	CMD('a', Mod_CONTROL, do_text_begin_line),
	CMD('j', Mod_CONTROL, do_back_char),
	CMD('l', Mod_CONTROL, do_forward_char),
};

void text_mode_init() {
	text_mode.name = "text";
	
	fox_for(i, 3) {
		register_cmd(&text_mode, text_cmds + i);
	}

	EditorState* state = get_editor_state();
	register_mode(state, &text_mode);
}
