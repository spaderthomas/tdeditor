static Mode text_mode;

void do_begin_line(Pane* pane);
void do_back_char(Pane* pane);
void do_forward_char(Pane* pane);
void do_delete_char_back(Pane* pane);
void do_new_line(Pane* pane);
void text_mode_init();

static Command text_cmds[] = {
	CMD('a', Mod_CONTROL, do_begin_line)
	CMD('j', Mod_CONTROL, do_back_char)
	CMD('l', Mod_CONTROL, do_forward_char)
	CMD(GLFW_KEY_BACKSPACE, Mod_NONE, do_delete_char_back)
	CMD(GLFW_KEY_ENTER, Mod_NONE, do_new_line)
	CMD_END()
};

