#ifndef TEXT_H
#define TEXT_H

#include <color.h>
#include <context.h>

#include <SDL.h>
#include <SDL_ttf.h>

typedef struct render_text {
    SDL_Surface* text_surface;
    SDL_Texture* copy_texture;
    TTF_Font* font;
    SDL_Color color;
    int width;
    int height;
} render_text_t;

render_text_t* text_new(const char* font, uint16_t font_size, const char* text, color_t color);
void text_draw(render_text_t* render_text, int screen_x, int screen_y);
void text_update(render_text_t* render_text, char* new_text);
void text_destroy(render_text_t* render_text);

#endif