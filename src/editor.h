struct DrawList;
typedef struct DrawList DrawList;

typedef struct EditorContext {
	int frame;
	tdstr contents;
	int cursor_idx;

	ScreenInfo screen_info;
	FontMap* fonts;

	DrawList* draw_list;
} EditorContext;

EditorContext* td_ctx();
void td_ctx_viewport_callback(EditorContext* ctx, IVec2 viewport);
void update(EditorContext* ctx);
void render(EditorContext* ctx);
void next_frame(EditorContext* ctx);
void draw_cursor(EditorContext* ctx);
void draw_text(EditorContext* ctx);
void register_font(EditorContext* ctx, char* font_path);
