#define true 1
#define false NULL
#define fox_for(iter, until) for (unsigned iter = 0; iter < until; iter++)
#define TDNS_LOG_ARR(sb) fox_for(i, sb_count((sb))) { TDNS_LOG(sb[i]); }

// Globals
uint32 g_VAO;
uint32 g_VBO;
uint32 EBO;
uint32 g_texture;
IVec2 g_viewport = { 800, 600 };

void TDNS_LOG(const char* str) {
	printf("%s", str);
}

// Returns the length of the string in bytes
uint32 file_length(FILE* file) {
	fseek(file, 0, SEEK_END);
	uint32 fsize = (uint32)ftell(file); // Not technically safe but who cares
	fseek(file, 0, SEEK_SET);

	return fsize;
}

char* file_contents(FILE* file) {
	uint32 flength = file_length(file);
	char* contents = (char*)malloc(flength + 1);
	fread(contents, flength, 1, file);
	contents[flength] = 0;

	return contents;
}

char* td_strcat(char* first, char* second) {
	int total_len = strlen(first) + strlen(second) + 1;
	char* buf = calloc(sizeof(char), total_len);
	
	memcpy(buf, first, strlen(first));
	memcpy(buf + strlen(first), second, strlen(second));
	
	return buf;
}

// dumb stretchy buffer: a mod of sean barrett's stretchy buffer that lets you push
// arbitrarily typed data to the same buffer. and since it's dumb, it keeps no metadata
// about its contents! push your own tags or make them a part of your structs. 
#define dsb_push(dsb, item) (dsb__maybegrow(dsb, sizeof(item)), memcpy(((char*)dsb + dsb_size(dsb)), &(item), sizeof(item)),  dsb__raw(dsb)[0] = dsb_size(dsb) + sizeof(item))
#define dsb_size(dsb) ((dsb) ? dsb__raw(dsb)[0] : 0)
#define dsb_cap(dsb) ((dsb) ? dsb__raw(dsb)[1] : 0)

#define dsb__raw(dsb) ((int*)(dsb) - 2)

#define dsb__needgrow(dsb, item) ((dsb)==0 || dsb_size(dsb) + sizeof(item) >= dsb_cap(dsb))
#define dsb__maybegrow(dsb, item) (dsb__needgrow(dsb, item) ? dsb__grow(dsb, item) : 0)
#define dsb__grow(dsb, item) (*((void**)&(dsb)) = dsb__growf(dsb, sizeof(item)))

void* dsb__growf(void* dsb, int bytes) {
	int double_cur = dsb ? dsb_cap(dsb) * 2 : 0;
	int min_needed = dsb_size(dsb) + bytes;
	int grow = double_cur > min_needed ? double_cur : min_needed;
	
	int* p = (int*)realloc(dsb ? dsb__raw(dsb) : 0, bytes + sizeof(int) * 2);
	if (!dsb) {
		p[0] = 0;
	}
	p[1] = grow;
	return p + 2;
}

static void glfw_error_callback(int code, const char* msg) {
	printf("Error %d: %s\n", code, msg);
}

void glfw_resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
	g_viewport.x = width;
	g_viewport.y = height;
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

struct Config {
	char** keys;
	char** values;
};
typedef struct Config Config;
Config g_config = {0};


void load_config() {
	FILE* config_file = fopen("/Users/thspader/Programming/tdeditor/src/tded.conf", "r");
	if (!config_file) {
		TDNS_LOG("Error opening the config file.");
	}
	char* config_source = file_contents(config_file);

	char** lines = NULL;
	char* line = strtok(config_source, "\n");
	while (line) {
		sb_push(lines, line);
		line = strtok(NULL, "\n");
	}

	fox_for(idx, sb_count(lines)) {
		char* key = strtok(lines[idx], " ");
		strtok(line, " "); // Eat the equals sign
		char* value = strtok(NULL, " ");

		int key_len = strlen(key);
		char* key_buf = calloc(key_len + 1, sizeof(char));
		memcpy(key_buf, key, key_len);
		sb_push(g_config.keys, key_buf);

		int val_len = strlen(value);
		char* val_buf = calloc(val_len + 1, sizeof(char));
		memcpy(val_buf, value, val_len);
		sb_push(g_config.values, val_buf);
	}
}

char* get_conf(char* key) {
	fox_for(idx, sb_count(g_config.keys)) {
		if (!strcmp(key, g_config.keys[idx])) {
			return g_config.values[idx];
		}
	}

	return NULL;
}
