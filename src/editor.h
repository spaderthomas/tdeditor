struct DrawList;
typedef struct DrawList DrawList;

struct Mode;
typedef struct Mode Mode;

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

	Mode** modes;             // stretchy buffer of all modes active in this buffer


	struct Buffer* next;
} Buffer;

typedef struct EditorState {
	int frame;

	Buffer* first_buffer;
	
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
void update();
void render();
void next_frame();
void draw_text(Buffer* ctx);
void draw_cursor(FontInfo* font, Vec2 point);
void handle_input(Buffer* buffer);

EditorState* state();
void editor_init(EditorState* state);

void buf_init(Buffer* buffer);
void buf_draw(Buffer* buffer);
Buffer* last_buffer();
Buffer* active_buffer();
Buffer* first_buffer();
void add_buffer();

void maybe_insert_char(Buffer* buffer, int key);
void maybe_insert_symbol(Buffer* buffer, int key);


void register_mode(EditorState* state, Mode* mode);
void register_cmd(Mode* mode, Command* cmd);
void activate_mode(Mode* mode, Buffer* buffer);

int td_buf_len(Buffer* buffer);
void td_delete_char_back(tdstr* buf, int i);
void td_new_line(tdstr* buf, int i);
