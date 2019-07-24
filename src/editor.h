struct DrawList;
typedef struct DrawList DrawList;

struct Mode;
typedef struct Mode Mode;

typedef struct Pane {
	float left;
	float right;
	float top;
	float bottom;
	int cursor_idx;
	tdstr contents;
	const char* name;

	bool active;

	Mode** modes;             // stretchy buffer of all modes active in this pane


	struct Pane* next;
} Pane;

typedef struct EditorState {
	int frame;

	Pane* pane;
	
	ScreenInfo screen_info;  
	FontMap* fonts;           // string hash map

	Mode** modes;             // stretchy buffer of all modes that have been registered
	DrawList* draw_list;      
} EditorState;

// GLFW doesn't have a generic for either of these. If a command defines its mod key as
// Mod_CONTROL, left or right control will work for it
typedef enum ModKey {
	Mod_NONE,
	Mod_CONTROL,
	Mod_META,
} ModKey;

typedef struct Command {
	uint16 key;
	ModKey mod;
	void (*func) (Pane*);
} Command;

typedef struct Mode {
	const char* name;
	Command** commands; // stretchy buffer
} Mode;

#define CMD(key, mod, fn) { key, mod, fn },
#define CMD_END() { 0, 0, NULL },
void update();
void render();
void next_frame();
void draw_text(Pane* ctx);
void handle_input(Pane* pane);

EditorState* state();
void editor_init(EditorState* state);
void pane_init(Pane* pane);

void register_mode(EditorState* state, Mode* mode);
void register_cmd(Mode* mode, Command* cmd);
void activate_mode(Mode* mode, Pane* pane);

int td_buf_len(Pane* pane);
void td_delete_char_back(tdstr* buf, int i);
void td_new_line(tdstr* buf, int i);
