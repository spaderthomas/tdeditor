// Everything to do with fonts
#define COUNT_ASCII 128

typedef struct FontChar {
	uint32 texture;
	Vec2 size;
	Vec2 bearing;
	float advance;
} FontChar;

typedef struct FontInfo {
	FontChar px_infos[COUNT_ASCII];
	FontChar screen_infos[COUNT_ASCII];
	float max_char_height;
} FontInfo;

typedef struct {
	char* key;
	FontInfo* value;
} FontMap;

void load_char_info_px(FT_Face face, FontInfo* font);;
void load_char_info_screen(FontInfo* font);
void load_default_font();
