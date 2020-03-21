struct DrawList;
typedef struct DrawList DrawList;

struct Mode;
typedef struct Mode Mode;

#define BUFFER_READABLE  1 << 0
#define BUFFER_WRITEABLE 1 << 1

typedef struct Buffer {
	float left;
	float right;
	float top;
	float bottom;
	
	int cursor_idx;
	tdstr contents;
	char* name;

	bool active;
	bool visible;
	
	int flags;

	Mode** modes;             // stretchy buffer of all modes active in this buffer

	struct Buffer* info;
	struct Buffer* next;
} Buffer;

typedef struct EditorState {
	int frame;

	Buffer* buf_first;
	
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
	void (*func) (Buffer*);
} Command;

typedef struct Mode {
	const char* name;
	Command** commands; // stretchy buffer
} Mode;

#define CMD(key, mod, fn) { key, mod, fn },
#define CMD_END() { 0, 0, NULL },

// Editor functions
EditorState* state();
void editor_init();
void update();
void render();
void next_frame();
void draw_text(Buffer* ctx);
void draw_cursor(FontInfo* font, Vec2 point);
void handle_input(Buffer* buffer);

// Buffer functions
void buf_init(Buffer* buffer);
void buf_draw(Buffer* buffer);
void buf_add();
void buf_add_info(Buffer* buffer);
Buffer* buf_last();
Buffer* buf_active();
Buffer* buf_first();

void maybe_insert_char(Buffer* buffer, int key);
void maybe_insert_symbol(Buffer* buffer, int key);

// Mode functions
void register_mode(EditorState* state, Mode* mode);
void register_cmd(Mode* mode, Command* cmd);
void activate_mode(Mode* mode, Buffer* buffer);


// API
void advance_point(Vec2* point, char c, FontInfo* font, float lbound, float rbound);
int td_buf_len(Buffer* buffer);
void td_delete_char_back(tdstr* buf, int i);
void td_new_line(tdstr* buf, int i);
void td_set_buf_contents(Buffer* buffer, tdstr contents);
