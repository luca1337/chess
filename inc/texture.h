#ifndef TEXTURE_H
#define TEXTURE_H

#include "context.h"
#include "color.h"

#include <SDL.h>

typedef struct texture{
    uint32_t width;
    uint32_t height;
    SDL_Texture* texture;
    SDL_Rect quad;
    SDL_Rect* clip;
    char* name;
    void(*render)(struct texture* texture, char use_alpha, uint8_t alpha, SDL_Rect* clip);
    void(*set_position)(struct texture* texture, int x, int y);
    void(*set_size)(struct texture* texture, float width, float height);
}texture_t;

texture_t* texture_load_from_file(const char* path, const char use_blending);
texture_t* texture_create_raw(uint32_t width, uint32_t height, color_t color);
void texture_destroy(texture_t* texture);

#endif