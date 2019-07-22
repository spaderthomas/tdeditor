struct {
	bool is_down[GLFW_KEY_LAST];
	bool was_down[GLFW_KEY_LAST];
} g_input;

char shift_map[128];
void fill_shift_map() {
	shift_map['a']  =  'A';
	shift_map['b']  =  'B';
	shift_map['c']  =  'C';
	shift_map['d']  =  'D';
	shift_map['e']  =  'E';
	shift_map['f']  =  'F';
	shift_map['g']  =  'G';
	shift_map['h']  =  'H';
	shift_map['i']  =  'I';
	shift_map['j']  =  'J';
	shift_map['k']  =  'K';
	shift_map['l']  =  'L';
	shift_map['m']  =  'M';
	shift_map['n']  =  'N';
	shift_map['o']  =  'O';
	shift_map['p']  =  'P';
	shift_map['q']  =  'Q';
	shift_map['r']  =  'R';
	shift_map['s']  =  'S';
	shift_map['t']  =  'T';
	shift_map['u']  =  'U';
	shift_map['v']  =  'V';
	shift_map['w']  =  'W';
	shift_map['x']  =  'X';
	shift_map['y']  =  'Y';
	shift_map['z']  =  'Z';
	shift_map['0']  =  ')';
	shift_map['1']  =  '!';
	shift_map['2']  =  '@';
	shift_map['3']  =  '#';
	shift_map['4']  =  '$';
	shift_map['5']  =  '%';
	shift_map['6']  =  '^';
	shift_map['7']  =  '&';
	shift_map['8']  =  '*';
	shift_map['9']  =  '(';
	shift_map['`']  =  '~';
	shift_map['-']  =  '_';
	shift_map['=']  =  '+';
	shift_map['[']  =  '{';
	shift_map['{']  =  '}';
	shift_map['\\'] =  '|';
	shift_map[';']  =  ':';
	shift_map['\''] =  '"';
	shift_map[',']  =  '<';
	shift_map['.']  =  '>';
	shift_map['/']  =  '?';
	shift_map[' ']  =  ' ';
}

// Since the GLFW callback handles marking is_down to be false, we
// don't need to reset them. Just have to make sure to mark when is_down
// values weren't input this frame
void reset_input() {
	fox_for(key, GLFW_KEY_LAST) {
		g_input.was_down[key] = g_input.is_down[key];
	}
}

bool was_pressed(int key) {
	return g_input.is_down[key] && !g_input.was_down[key];
}

bool is_key_down(int key) {
	return g_input.is_down[key];
}

bool is_shift_down() {
	return g_input.is_down[GLFW_KEY_LEFT_SHIFT] || g_input.is_down[GLFW_KEY_RIGHT_SHIFT];
}

bool was_ctrl_pressed() {
	return was_pressed(GLFW_KEY_LEFT_CONTROL) || was_pressed(GLFW_KEY_RIGHT_CONTROL);
}

bool is_ctrl_down() {
	return g_input.is_down[GLFW_KEY_LEFT_CONTROL] || g_input.is_down[GLFW_KEY_RIGHT_CONTROL];
}

bool is_meta_down() {
	return g_input.is_down[GLFW_KEY_LEFT_ALT] || g_input.is_down[GLFW_KEY_RIGHT_ALT];
}

bool is_mod_key_down(ModKey key) {
	if (key == Mod_NONE) {
		return true;
	}
	else if (key == Mod_CONTROL) {
		return is_ctrl_down();
	}
	else if (key == Mod_META) {
		return is_meta_down();
	}
	
	return false;
}

bool is_any_mod_key_down() {
	return is_ctrl_down() || is_meta_down();
}
