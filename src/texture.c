#include "texture.h"

#include <string.h>
#include <stdlib.h>
#include <Windows.h>

// do not change this order
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

extern renderer_t *renderer;

/*static void _load_from_file(const char *path, struct texture *out_texture)
{
    int tex_width, tex_height, channels_count;
    unsigned char *bitmap = stbi_load(path, &tex_width, &tex_height, &channels_count, STBI_rgb_alpha);
    if (!bitmap)
    {
        SDL_Log("Unable to load texture");
        return;
    }

    out_texture->texture = SDL_CreateTexture((SDL_Renderer *)renderer->sdl_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, tex_width, tex_height);

    int pitch = 0;
    unsigned char *pixels = NULL;

    // temporary lock texture for writing pixels
    if (SDL_LockTexture(out_texture->texture, NULL, (void **)&pixels, &pitch))
    {
        SDL_Log("Unable to lock texture: %s", SDL_GetError());
        return;
    }

    memset(pixels, 0, tex_width * tex_height * channels_count);
    memcpy(pixels, bitmap, tex_width * tex_height * channels_count);
    SDL_SetTextureBlendMode(out_texture->texture, SDL_BLENDMODE_BLEND);
    SDL_UnlockTexture(out_texture->texture);

    out_texture->width = tex_width;
    out_texture->height = tex_height;
    out_texture->quad.w = tex_width;
    out_texture->quad.h = tex_height;
}*/

void _render(struct texture *texture, char use_alpha, uint8_t alpha, SDL_Rect *clip)
{
    SDL_Renderer *native_renderer = (SDL_Renderer *)renderer->sdl_renderer;

    if (!texture)
    {
        return;
    }

    SDL_SetRenderDrawBlendMode(native_renderer, SDL_BLENDMODE_BLEND);
    if (use_alpha)
        SDL_SetTextureAlphaMod(texture->texture, alpha);
    SDL_RenderCopy(native_renderer, texture->texture, NULL, &texture->quad);
}

void _set_position(struct texture *texture, int x, int y)
{
    texture->quad.x = x;
    texture->quad.y = y;
}

void _set_size(struct texture *texture, float width, float height)
{
    texture->quad.w = width;
    texture->quad.h = height;
}

texture_t *texture_create_raw(uint32_t width, uint32_t height, color_t color)
{
    texture_t *texture = (texture_t*)calloc(1, sizeof(texture_t));
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

    for (int y = 0; y < width; ++y)
    {
        for (int x = 0; x < height; ++x)
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

texture_t *texture_load_from_file(const char *path)
{
    texture_t *texture = (texture_t*)calloc(1, sizeof(texture_t));
    texture->render = _render;
    texture->set_position = _set_position;
    texture->set_size = _set_size;

    int width, height, color_channel;
    unsigned char *data = stbi_load(path, &width, &height, &color_channel, STBI_rgb_alpha);

    // SDL_RendererInfo info;
    // SDL_GetRendererInfo( (SDL_Renderer *)renderer->sdl_renderer, &info );
    // printf("Renderer name: %s\n", info.name);
    // printf("Renderer formats\n");
    // for( Uint32 i = 0; i < info.num_texture_formats; i++ )
    // {
    //     printf(" %s ", SDL_GetPixelFormatName( info.texture_formats[i]));
    // }

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
    SDL_SetTextureBlendMode(texture->texture, SDL_BLENDMODE_BLEND);
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
        texture = NULL;
    }
}