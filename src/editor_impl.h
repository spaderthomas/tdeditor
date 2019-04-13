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

void update(EditorContext* ctx) {
	// Move the cursor
	if (was_pressed(GLFW_KEY_RIGHT)) ctx->cursor_idx++;
	if (was_pressed(GLFW_KEY_LEFT)) ctx->cursor_idx--;

	// Move the input into the text buffer
	send_input(&ctx->contents);

	// Emit commands to draw the text + cursor
	draw_text(ctx);
	draw_cursor(ctx);
	
    ctx->frame++;
}

void next_frame(EditorContext* ctx) {
	dl_reset(ctx->draw_list);
}

void render(EditorContext* ctx) {
	dl_render(ctx->draw_list);
}

void draw_cursor(EditorContext* ctx) {
	FontInfo* font = shget(ctx->fonts, get_conf("font_default"));
	
	Vec2 point = { -1.f, 1.f - font->max_char_height };
	fox_for(idx, ctx->contents.len) {
		char c = ctx->contents.buf[idx];
		FontChar* char_info = &font->screen_infos[c];
		
		point.x += char_info->advance;
		if (point.x > 1) {
			point.x = -1;
			point.y -= font->max_char_height;
		}

		if (idx == ctx->cursor_idx - 1) {
			Vertex cursor_top = {
								 {point.x, point.y},
								 {0.f, 0.f, 0.f},
								 {1.f, 0.f}
			};

			Vertex cursor_bottom = {
									{point.x, point.y + font->max_char_height},
									{0.f, 0.f, 0.f},
									{1.f, 1.f}
			};

			Vertex cursor_verts[2] = {
									  cursor_top,
									  cursor_bottom
			};

			uint32 cursor_elems[2] = {
									  0, 1
			};

			DrawCommand cmd;
			cmd.count_elems = 2;
			cmd.count_verts = 2;
			cmd.mode = GL_LINES;
			make_basic_shader_info(shader_info);
			cmd.shader_info = shader_info;
			dl_push_primitive(ctx->draw_list,
							  cursor_verts, cursor_elems,
							  &cmd);
			
			return;
		}
	}

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
		float bottom = point.y - char_info->size.y + char_info->bearing.y;
		float top = point.y + char_info->bearing.y;
		float left = point.x + char_info->bearing.x;
		float right = point.x + char_info->bearing.x + char_info->size.x;

		Vertex bottom_left = {
		   {left, bottom},
		   {1.f, 1.f, 1.f},
		   {0.f, 1.f}
		};
		Vertex bottom_right = {
			{right, bottom},
			{1.f, 1.f, 1.f},
			{1.f, 1.f}
		};
		Vertex top_left = {
    		{left, top},
	    	{1.f, 1.f, 1.f},
    		{0.f, 0.f}
		};
		Vertex top_right = {
		    {right, top},
		    {1.f, 1.f, 1.f},
		    {1.f, 0.f}
		};

		Vertex verts[4] = {
		    top_right,
		    bottom_right,
		    bottom_left,
		    top_left
	    };
		
	    uint32 indices[6] = {
	    	0, 1, 3,
	    	1, 2, 3
    	};
		
		DrawCommand draw_cmd;
		draw_cmd.count_elems = 6;
		draw_cmd.count_verts = 4;
		draw_cmd.mode = GL_TRIANGLES;
		make_text_shader_info(shader_info);
		shader_info.text.texture = char_info->texture;
		draw_cmd.shader_info = shader_info;

		dl_push_primitive(ctx->draw_list, verts, indices, &draw_cmd);

		point.x += char_info->advance;
		if (point.x > 1) {
			point.x = -1;
			point.y -= font->max_char_height;
		}
	}
}

