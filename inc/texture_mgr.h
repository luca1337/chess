#ifndef TEXTURE_MGR_H
#define TEXTURE_MGR_H

#include "context.h"
#include "texture.h"
#include "tex_list.h"

typedef struct texture_mgr{
    tex_list_t* textures;
}texture_mgr_t;

texture_mgr_t* texture_mrg_new();
texture_t* add_texture(texture_mgr_t*, const char*, const char*, uint32_t, int, int);
texture_t* get_texture(texture_mgr_t*, const char*);
void destroy_texture_mgr(texture_mgr_t*);

#endif