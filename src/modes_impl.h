void do_begin_line(Buffer* buffer) {
	printf("do_begin_line\n");
	buffer->cursor_idx = 0;
}

void do_back_char(Buffer* buffer) {
	printf("do_back_char\n");
	buffer->cursor_idx = tdmax(buffer->cursor_idx - 1, 0);
}

void do_forward_char(Buffer* buffer) {
	printf("do_forward_char\n");
	buffer->cursor_idx = tdmin(buffer->cursor_idx + 1, td_buf_len(buffer));
}

void do_delete_char_back(Buffer* buffer) {
	printf("do_delete_char_back\n");
	td_delete_char_back(&buffer->contents, buffer->cursor_idx - 1);
	buffer->cursor_idx = tdmax(buffer->cursor_idx - 1, 0);
}

void do_new_line(Buffer* buffer) {
	printf("do_new_line\n");
	td_new_line(&buffer->contents, buffer->cursor_idx);
	buffer->cursor_idx = tdmin(buffer->cursor_idx + 1, td_buf_len(buffer));
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
}


/*
 * Fundamental Mode
 */
void do_other_buffer(Buffer* buffer) {
	printf("do_other_buffer\n");
	Buffer* old_active = buf_active();
	old_active->active = false;
	
	if (!old_active->next) {
		state()->buf_first->active = true;
		return;
	}
	old_active->next->active = true;
}

void do_split_vertical(Buffer* buffer) {
	printf("do_split_vertical\n");
	Buffer* new_buffer = calloc(1, sizeof(Buffer));
	buf_copy(new_buffer, buffer);

	buffer->active = false;

	buffer->next = new_buffer;
	
	// By default, the new buffer goes to the left and the old one to the right
	float boundary = (buffer->left + buffer->right) / 2;
	new_buffer->left = boundary;
	buffer->right = boundary;
	if (buffer->info) buffer->info->right = boundary;
	if (new_buffer->info) new_buffer->info->left = boundary;

	activate_mode(&text_mode, new_buffer);
	activate_mode(&fundamental_mode, new_buffer);
}

void do_find_file(Buffer* buffer) {
	TD_LOG("do_find_file");
	
}


void fundamental_mode_init() {
	fundamental_mode.name = "fundamental";

	Command* cmd = fundamental_cmds;
	while(*((int*)cmd)) {
		register_cmd(&fundamental_mode, cmd);
		cmd++;
	}

	register_mode(state(), &fundamental_mode);
}
