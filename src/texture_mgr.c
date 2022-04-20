#include "texture_mgr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <stb_image.h>

texture_mgr_t* texture_mrg_new()
{
    texture_mgr_t* texture_mgr = malloc(sizeof(texture_mgr_t));

    memset(texture_mgr, 0, sizeof(texture_mgr_t));

    texture_mgr->textures = tex_list_new(NULL);

    return texture_mgr;
}

texture_t* add_texture(texture_mgr_t* t_mgr, const char* key, const char* path)
{
    texture_t* texture = texture_load_from_file(path, 1);

    size_t len = strlen(key);
    char* buf = malloc(len + 1);
    strcpy_s(buf, len + 1, key);

    texture->name = buf;

    tex_list_append(t_mgr->textures, texture);

    return texture;
}

texture_t* get_texture(texture_mgr_t* mgr, const char* key)
{
    texture_t* tex = tex_list_get(mgr->textures, key);
    if (!tex)
    {
        printf("something went wrong\n");
        return NULL;
    }
    return tex;
}

void destroy_texture_mgr(texture_mgr_t* mgr) { tex_list_destroy(mgr->textures); }