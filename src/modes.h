static Mode text_mode;

void do_begin_line(Buffer* buffer);
void do_back_char(Buffer* buffer);
void do_forward_char(Buffer* buffer);
void do_delete_char_back(Buffer* buffer);
void do_new_line(Buffer* buffer);
void text_mode_init();

static Command text_cmds[] = {
	CMD('a', Mod_CONTROL, do_begin_line)
	CMD('j', Mod_CONTROL, do_back_char)
	CMD('l', Mod_CONTROL, do_forward_char)
	CMD(GLFW_KEY_BACKSPACE, Mod_NONE, do_delete_char_back)
	CMD(GLFW_KEY_ENTER, Mod_NONE, do_new_line)
	CMD_END()
};


static Mode fundamental_mode;

void do_other_buffer(Buffer* buffer);
void do_split_vertical(Buffer* buffer);
void do_find_file(Buffer* buffer);

static Command fundamental_cmds[] = {
	CMD('b', Mod_CONTROL, do_other_buffer)
	CMD('3', Mod_CONTROL, do_split_vertical)
	CMD('f', Mod_CONTROL, do_find_file)
	CMD_END()
};
