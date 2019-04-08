struct {
	bool is_down[GLFW_KEY_LAST];
	bool was_down[GLFW_KEY_LAST];
} g_input;

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

void send_input(tdstr buffer) {
	
}
