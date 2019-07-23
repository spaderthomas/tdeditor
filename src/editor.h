struct DrawList;
typedef struct DrawList DrawList;

struct Mode;
typedef struct Mode Mode;

typedef struct EditorState {
	int frame;
	int cursor_idx;

	tdstr contents;
	
	ScreenInfo screen_info;  
	FontMap* fonts;           // string hash map

	Mode** modes;             // stretchy buffer
	DrawList* draw_list;      
} EditorState;

// GLFW doesn't have a generic for either of these. If a command defines its mod key as
// Mod_NONE, left or right control will work for it
typedef enum ModKey {
	Mod_NONE,
	Mod_CONTROL,
	Mod_META,
} ModKey;

typedef struct Command {
	uint16 key;
	ModKey mod;
	void (*func) (EditorState*);
} Command;

typedef struct Mode {
	const char* name;
	Command** commands; // stretchy buffer
} Mode;

#define CMD(key, mod, fn) { key, mod, fn },
#define CMD_END() { 0, 0, NULL },
EditorState* get_editor_state();
void update();
void render();
void next_frame();
void draw_text(EditorState* ctx);
void handle_input(tdstr* buffer);

void register_mode(EditorState* state, Mode* mode);
void register_cmd(Mode* mode, Command* cmd);

int td_buf_len(EditorState* state);
void td_delete_char_back(tdstr* buf, int i);
void td_new_line(tdstr* buf, int i);
