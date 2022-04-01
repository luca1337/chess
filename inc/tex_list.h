#ifndef TEX_LIST_H
#define TEX_LIST_H

typedef struct tex_list_item{
    void* tex;
    struct tex_list_item* next;
    struct tex_list_item* prev;
}tex_list_item_t;

tex_list_item_t* tex_list_item_new(int*);
void tex_list_item_destroy(tex_list_item_t*);

typedef struct tex_list{
    tex_list_item_t* head;
    tex_list_item_t* tail;
    long long unsigned count;
}tex_list_t;

tex_list_t* tex_list_new(int*);
int tex_list_append(tex_list_t*, void*);
void* tex_list_get(tex_list_t*, const char*);
void tex_list_destroy(tex_list_t*);

#endif