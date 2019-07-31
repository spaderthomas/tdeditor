void do_begin_line(Pane* pane) {
	printf("do_begin_line\n");
	pane->cursor_idx = 0;
}

void do_back_char(Pane* pane) {
	printf("do_back_char\n");
	pane->cursor_idx = tdmax(pane->cursor_idx - 1, 0);
}

void do_forward_char(Pane* pane) {
	printf("do_forward_char\n");
	pane->cursor_idx = tdmin(pane->cursor_idx + 1, td_buf_len(pane));
}

void do_delete_char_back(Pane* pane) {
	printf("do_delete_char_back\n");
	td_delete_char_back(&pane->contents, pane->cursor_idx - 1);
	pane->cursor_idx = tdmax(pane->cursor_idx - 1, 0);
}

void do_new_line(Pane* pane) {
	printf("do_new_line\n");
	td_new_line(&pane->contents, pane->cursor_idx);
	pane->cursor_idx = tdmin(pane->cursor_idx + 1, td_buf_len(pane));
}

void do_load_file(Pane* pane) {
	printf("do_load_file\n");
}

void text_mode_init() {
	text_mode.name = "text";

	// Loop through commands until we find CMD_END, registering each command
	Command* cmd = text_cmds;
	while(*((int*)cmd)) {
		register_cmd(&text_mode, cmd);
		cmd++;
	}

	register_mode(state(), &text_mode);
	activate_mode(&text_mode, state()->first_pane);
}


/*
 * Fundamental Mode
 */
void do_other_buffer(Pane* pane) {
	printf("do_other_buffer\n");
	Pane* old_active = active_pane();
	old_active->active = false;
	
	if (!old_active->next) {
		state()->first_pane->active = true;
		return;
	}
	old_active->next->active = true;
}

void fundamental_mode_init() {
	fundamental_mode.name = "fundamental";

	Command* cmd = fundamental_cmds;
	while(*((int*)cmd)) {
		register_cmd(&fundamental_mode, cmd);
		cmd++;
	}

	register_mode(state(), &fundamental_mode);
	activate_mode(&fundamental_mode, state()->first_pane);
}
