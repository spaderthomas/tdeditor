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

bool is_shift_down() {
	return g_input.is_down[GLFW_KEY_LEFT_SHIFT] || g_input.is_down[GLFW_KEY_RIGHT_SHIFT];
}

// Applies modifiers and then sends the characters to the display buffer
void send_input(tdstr* buffer) {
	// Have to handle this loop in three contiguous segments:
	// 1. All the keys before letters
	for(int key = GLFW_KEY_SPACE; key <= GLFW_KEY_EQUAL; key++) {
		if (was_pressed(key)) {
			if (is_shift_down()) {
				tdstr_push(buffer, shift_map[key]);
			} else {
				tdstr_push(buffer, key);
				
			}
		}
	}

	// 2. Characters, so we can subtract off to get lower case
	for(int key = GLFW_KEY_A; key <= GLFW_KEY_Z; key++) {
		if (was_pressed(key)) {
			// Upper case
			if (is_shift_down()) {
				tdstr_push(buffer, key);
			}
			// Lower case
			else {
				tdstr_push(buffer, key + 32);
				
			}
		}
	}

	// 3. All the keys after letters
	for(int key = GLFW_KEY_LEFT_BRACKET; key <= GLFW_KEY_GRAVE_ACCENT; key++) {
		if (was_pressed(key)) {
			if (is_shift_down()) {
				tdstr_push(buffer, shift_map[key]);
			} else {
				tdstr_push(buffer, key);
				
			}
		}
	}

}

