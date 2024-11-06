#include <private.h>
#include <texture.h>

#include <Windows.h>
#include <stdlib.h>
#include <string.h>

// do not change this order
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

extern renderer_t *renderer;

void _render(struct texture *texture, uint8_t alpha, SDL_Rect *clip)
{
    SDL_Renderer *native_renderer = (SDL_Renderer *)renderer->sdl_renderer;

    if (!texture)
    {
        return;
    }

    if (alpha > 0) SDL_SetTextureAlphaMod(texture->texture, alpha);

    SDL_RenderCopy(native_renderer, texture->texture, NULL, &texture->quad);
}

void _set_position(struct texture *texture, int x, int y)
{
    texture->quad.x = x;
    texture->quad.y = y;
}

void _set_size(struct texture *texture, int width, int height)
{
    texture->quad.w = width;
    texture->quad.h = height;
}

texture_t *texture_create_raw(uint32_t width, uint32_t height, color_t color)
{
    texture_t *texture = (texture_t *)calloc(1, sizeof(texture_t));
    CHECK(texture, NULL, "Couldn't allocate memory for struct texture");
    texture->render = _render;
    texture->set_position = _set_position;
    texture->set_size = _set_size;

    texture->texture = SDL_CreateTexture((SDL_Renderer *)renderer->sdl_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);

    if (!texture->texture)
    {
        SDL_Log("could not create texture: %s", SDL_GetError());
        return NULL;
    }

    int pitch = 0;
    unsigned char *pixels = NULL;

    if (SDL_LockTexture(texture->texture, NULL, (void **)&pixels, &pitch))
    {
        SDL_Log("unable to lock texture: %s", SDL_GetError());
        return NULL;
    }

    unsigned char colors[4] = {color.r, color.g, color.b, color.a};

    for (uint32_t y = 0; y != width; ++y)
    {
        for (uint32_t x = 0; x != height; ++x)
        {
            memcpy(&pixels[(y * height + x) * sizeof(color)], colors, sizeof(color));
        }
    }

    SDL_SetTextureBlendMode(texture->texture, SDL_BLENDMODE_BLEND);
    SDL_UnlockTexture(texture->texture);

    texture->width = width;
    texture->height = height;
    texture->quad.w = width;
    texture->quad.h = height;

    return texture;
}

texture_t *texture_load_from_file(const char *path, const char use_blending)
{
    texture_t *texture = (texture_t *)calloc(1, sizeof(texture_t));
    CHECK(texture, NULL, "Couldn't allocate memory for struct texture");
    texture->render = _render;
    texture->set_position = _set_position;
    texture->set_size = _set_size;

    int width, height, color_channel;
    unsigned char *data = stbi_load(path, &width, &height, &color_channel, STBI_rgb_alpha);

    texture->texture = SDL_CreateTexture((SDL_Renderer *)renderer->sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    int pitch = 0;
    unsigned char *pixel = NULL;

    if (SDL_LockTexture(texture->texture, NULL, (void **)&pixel, &pitch))
    {
        SDL_Log("Unable to lock texture: %s", SDL_GetError());
        return NULL;
    }

    memset(pixel, 0, width * height * color_channel);
    memcpy(pixel, data, width * height * color_channel);
    if (use_blending) SDL_SetTextureBlendMode(texture->texture, SDL_BLENDMODE_BLEND);
    SDL_UnlockTexture(texture->texture);

    texture->width = width;
    texture->height = height;
    texture->quad.w = width;
    texture->quad.h = height;

    return texture;
}

void texture_destroy(texture_t *texture)
{
    if (texture)
    {
        SDL_DestroyTexture(texture->texture);
        SDL_free(texture->texture);
        free(texture);
    }
}