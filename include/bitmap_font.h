#ifndef BITMAP_FONT_H
#define BITMAP_FONT_H

#include <SDL.h>
#include <stdbool.h>
#include <stddef.h>

#define FONT_CHAR_WIDTH 5
#define FONT_CHAR_HEIGHT 7
#define BITMAP_FONT_SCALE 3

typedef struct {
    int glyphWidth;
    int glyphHeight;
    int glyphSpacing;
} BitmapFont;

bool bitmap_font_init(BitmapFont *font);
void bitmap_font_shutdown(BitmapFont *font);
void bitmap_font_draw_text(SDL_Renderer *renderer, const BitmapFont *font, int x, int y, const char *text, SDL_Color color);
int bitmap_font_measure_text(const BitmapFont *font, const char *text);

#endif // BITMAP_FONT_H
