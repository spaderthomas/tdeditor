EditorContext* td_ctx() {
	static EditorContext* ctx = NULL;
	if (!ctx) {
		ctx = calloc(1, sizeof(EditorContext));
		ctx->frame = 0;
		tdstr_init(&ctx->contents);
		ctx->cursor_idx = 0;
		ctx->screen_info.viewport.x = 1920;
		ctx->screen_info.viewport.y = 1080;
		ctx->draw_list = calloc(1, sizeof(DrawList));
		ctx->fonts = NULL;
	}

	return ctx;
}

void td_ctx_viewport_callback(EditorContext* ctx, IVec2 viewport) {
	// Match the context's viewport with the new value
	ctx->screen_info.viewport = viewport;

	// Remap the pixel values of loaded font into screen values
	FontInfo* font_info = shget(ctx->fonts, get_conf("font_default"));
	load_char_info_screen(font_info);
}

void update() {
	EditorContext* ctx = td_ctx();
	
	// Move the cursor
	if (was_pressed(GLFW_KEY_RIGHT)) ctx->cursor_idx++;
	if (was_pressed(GLFW_KEY_LEFT)) ctx->cursor_idx--;

	// Move the input into the text buffer
	send_input(&ctx->contents);

	// Emit commands to draw the text + cursor
	draw_text(ctx);
	
    ctx->frame++;
}

void next_frame() {
	EditorContext* ctx = td_ctx();
	dl_reset(ctx->draw_list);
}

void render() {
	EditorContext* ctx = td_ctx();
	dl_render(ctx->draw_list);
}


void draw_text(EditorContext* ctx) {
	FontInfo* font = shget(ctx->fonts, get_conf("font_default"));
	char* text = ctx->contents.buf;
	uint32 len = strlen(text);
	
	Vec2 point = { -1.f, 1.f - font->max_char_height };
	fox_for(idx, len) {
		char c = text[idx];
		if (c == '\n') {
			point.x = -1;
			point.y -= font->max_char_height;
		}
		
		FontChar* char_info = &font->screen_infos[c];

		// Make a rectangle:
		float top = point.y + char_info->bearing.y;
		float bottom = point.y - char_info->size.y + char_info->bearing.y;
		float left = point.x + char_info->bearing.x;
		float right = point.x + char_info->bearing.x + char_info->size.x;

		DrawCommand cmd;
		cmd.shader_info.text.shader = text_shader;
		cmd.shader_info.text.texture = char_info->texture;
		dl_push_rect(ctx->draw_list, top, bottom, left, right, &cmd);

		point.x += char_info->advance;
		if (point.x > 1) {
			point.x = -1;
			point.y -= font->max_char_height;
		}
	}

	Vec2 a = { point.x, point.y };
	Vec2 b = { point.x, point.y + font->max_char_height };
	DrawCommand cmd;
	cmd.shader_info.basic.shader = basic_shader;
	dl_push_line(ctx->draw_list, a, b, &cmd);
}

