// Call this once to generate a texture for each character and mark the
// pixel values that define the character
void load_char_info_px(FT_Face face, FontInfo* font) {
	for (char c = ' '; c <= '~'; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			char* error_msg = malloc(64 * sizeof(char));
			sprintf(error_msg, "FreeType failed to load character: %c", c);
			TD_LOG(error_msg);
		}

		FontChar* px_info = font->px_infos + c;
		glGenTextures(1, &px_info->texture);
		glBindTexture(GL_TEXTURE_2D, px_info->texture);
		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 GL_RED,
					 face->glyph->bitmap.width, face->glyph->bitmap.rows,
					 0,
					 GL_RED,
					 GL_UNSIGNED_BYTE,
					 face->glyph->bitmap.buffer);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		px_info->size.x = face->glyph->bitmap.width;
		px_info->size.y = face->glyph->bitmap.rows;
		px_info->bearing.x = face->glyph->bitmap_left;
		px_info->bearing.y = face->glyph->bitmap_top;
		px_info->advance = (float)face->glyph->advance.x / 64; // FT stores this in 1/64 pixel increments
	}
}

// Convenience: Translate the pixel values that FreeType gives us into
// screen coordinates. Call this function when the viewport changes
void load_char_info_screen(FontInfo* font) {
	EditorState* ctx = state();
	
	for (char c = ' '; c <= '~'; c++) {
		FontChar* screen_info = font->screen_infos + c;
		FontChar* px_info = font->px_infos + c;
		
		screen_info->texture   = px_info->texture;
		screen_info->size.x    = px_info->size.x    / (float)ctx->screen_info.viewport.x;
		screen_info->size.y    = px_info->size.y    / (float)ctx->screen_info.viewport.y;
		screen_info->bearing.x = px_info->bearing.x / (float)ctx->screen_info.viewport.x;
		screen_info->bearing.y = px_info->bearing.y / (float)ctx->screen_info.viewport.y;
		screen_info->advance   = px_info->advance   / (float)ctx->screen_info.viewport.x;

		// Cache the tallest character in screen units because it's useful
		font->max_char_height = tdmax(font->max_char_height, px_info->size.y / ctx->screen_info.viewport.y);
	}

}

void load_default_font(EditorState* ctx) {
	// Initialize FreeType
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		TD_LOG("Error initializing FreeType");
	}

	// Get the default font
	FT_Face face;
	char* path = td_strcat(font_path, default_font);
	if (FT_New_Face(ft, path, 0, &face)) {
		TD_LOG("Error loading font");
		TD_LOG(td_strcat("Looked for font in: ", path));
	}
	free(path);
	

	// Set FT + GL configs
	FT_Set_Pixel_Sizes(face, 0, 48);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Parse the font into a C struct
	FontInfo* font = calloc(1, sizeof(FontInfo));
	load_char_info_px(face, font);
	load_char_info_screen(font);
	shput(ctx->fonts, default_font, font);
}
