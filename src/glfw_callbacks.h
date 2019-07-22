void glfw_error_callback(int code, const char* msg) {
	printf("Error %d: %s\n", code, msg);
}

void glfw_resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
	EditorState* ctx = get_editor_state();
	IVec2 viewport;
	viewport.x = width;
	viewport.y = height;

	// Match the context's viewport with the new value
	ctx->screen_info.viewport = viewport;

	// Remap the pixel values of loaded font into screen values
	FontInfo* font_info = shget(ctx->fonts, get_conf("font_default"));
	load_char_info_screen(font_info);

}


void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) { g_input.is_down[key] = true; }
    if (action == GLFW_RELEASE) { g_input.is_down[key] = false; } 
}

void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void *userParam) {
	(void)userParam;

	switch (id) {
	case 131169: // The driver allocated storage for renderbuffer
		return;
	case 131185: // glBufferData
		return;
	case 481131: // buffer info
		return;
	case 131184: // buffer info
		return;
	case 131204: // Weird base level inconsistent bug. It may be a driver bug.
		return;
	}

	char** debug_msg = NULL;
	
	sb_push(debug_msg, "OpenGL Debug Message: ");
	sb_push(debug_msg, "\nSource: ");
	switch (source) {
	case GL_DEBUG_SOURCE_API:
		sb_push(debug_msg, "API");
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		sb_push(debug_msg, "Window System");
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		sb_push(debug_msg, "Shader Compiler");
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		sb_push(debug_msg, "Third Party");
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		sb_push(debug_msg, "Application");
		break;
	case GL_DEBUG_SOURCE_OTHER:
		sb_push(debug_msg, "Other");
		break;
	}

	sb_push(debug_msg, "\nType: ");
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		sb_push(debug_msg, "Error");
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		sb_push(debug_msg, "Deprecated Behaviour");
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		sb_push(debug_msg, "Undefined Behaviour");
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		sb_push(debug_msg, "Portability");
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		sb_push(debug_msg, "Performance");
		break;
	case GL_DEBUG_TYPE_MARKER:
		sb_push(debug_msg, "Marker");
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		sb_push(debug_msg, "Push Group");
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		sb_push(debug_msg, "Pop Group");
		break;
	case GL_DEBUG_TYPE_OTHER:
		sb_push(debug_msg, "Other");
		break;
	}

	sb_push(debug_msg, "\nID: ");
	char* id_str = malloc(16 * sizeof(char));
	sprintf(id_str, "%d", id);
	sb_push(debug_msg, id_str);
	
	sb_push(debug_msg, "\nSeverity: ");
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		sb_push(debug_msg, "High");
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		sb_push(debug_msg, "Medium");
		break;
	case GL_DEBUG_SEVERITY_LOW:
		sb_push(debug_msg, "Low");
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		sb_push(debug_msg, "Notification");
		break;
	}

	sb_push(debug_msg, "\nGL message: ");
	char* msg_copy = malloc(strlen(message) * strlen(message));
	strcpy(msg_copy, message);
	sb_push(debug_msg, msg_copy);
	sb_push(debug_msg, "\n\n");

	TDNS_LOG_ARR(debug_msg);
}
