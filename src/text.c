#include "text.h"
#include "private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern renderer_t *renderer;

render_text_t *text_new(const char *font, uint16_t font_size, const char *text, color_t color)
{
    render_text_t *render_text = (render_text_t *)calloc(1, sizeof(render_text_t));
    CHECK(render_text, NULL, "Could not allocate enough bytes for struct text");

    render_text->font = TTF_OpenFont(font, font_size);

    SDL_Color text_color = {color.r, color.g, color.b, color.a};
    render_text->text_surface = TTF_RenderText_Blended(render_text->font, text, text_color);
    render_text->copy_texture = SDL_CreateTextureFromSurface((SDL_Renderer *)renderer->sdl_renderer, render_text->text_surface);

    render_text->color = text_color;

    SDL_QueryTexture(render_text->copy_texture, NULL, NULL, &render_text->width, &render_text->height);

    return render_text;
}

void text_draw(render_text_t *render_text, int screen_x, int screen_y)
{
    SDL_assert_always(render_text);

    SDL_Rect dstrect = {screen_x, screen_y, render_text->width, render_text->height};
    SDL_RenderCopy((SDL_Renderer *)renderer->sdl_renderer, render_text->copy_texture, NULL, &dstrect);
}

void text_update(render_text_t *render_text, char *new_text)
{
    SDL_FreeSurface(render_text->text_surface);
    SDL_DestroyTexture(render_text->copy_texture);

    render_text->text_surface = NULL;
    render_text->copy_texture = NULL;

    render_text->text_surface = TTF_RenderText_Blended(render_text->font, new_text, render_text->color);
    render_text->copy_texture = SDL_CreateTextureFromSurface((SDL_Renderer *)renderer->sdl_renderer, render_text->text_surface);

    // also update width and height that might be changed if the new text is wider or smaller
    SDL_QueryTexture(render_text->copy_texture, NULL, NULL, &render_text->width, &render_text->height);
}

void text_destroy(render_text_t *render_text)
{
    SDL_FreeSurface(render_text->text_surface);
    SDL_DestroyTexture(render_text->copy_texture);
    TTF_CloseFont(render_text->font);
    free(render_text);
}