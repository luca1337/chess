#include <tex_list.h>
#include <texture.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tex_list_item_t* tex_list_item_new(int* err)
{
    tex_list_item_t* item = malloc(sizeof(tex_list_item_t));
    memset(item, 0, sizeof(*item));
    return item;
}

void tex_list_item_destroy(tex_list_item_t* item)
{
    // todo implement destroy
    free(item->next);
    free(item->prev);
    free(item);
    item = NULL;
}

tex_list_t* tex_list_new(int* err)
{
    tex_list_t* list = malloc(sizeof(tex_list_t));
    memset(list, 0, sizeof(*list));
    return list;
}

int tex_list_append(tex_list_t* list, void* tex)
{
    tex_list_item_t* current = list->head;

    while (current)
    {
        texture_t* tx0 = (texture_t*)tex;
        texture_t* tx1 = (texture_t*)current->tex;

        if (!strcmp(tx0->name, tx1->name))
        {
            fprintf(stderr, "texture already exists!\n");
            return -1;
        }

        current = current->next;
    }

    tex_list_item_t* item = tex_list_item_new(NULL);

    if (!list->head)
    {
        list->head = item;
        list->tail = item;
        item->tex = tex;
        list->count++;
        return 0;
    }

    list->tail->next = item;
    item->prev = list->tail;
    list->tail = item;
    item->next = NULL;
    item->tex = tex;
    list->count++;

    return 0;
}

void* tex_list_get(tex_list_t* list, const char* key)
{
    tex_list_item_t* current = list->head;
    texture_t* tex = NULL;
    while (current)
    {
        texture_t* tx = (texture_t*)current->tex;

        if (strcmp(tx->name, key) == 0)
        {
            tex = current->tex;
            return tex;
        }

        current = current->next;
    }
    return tex;
}

void tex_list_destroy(tex_list_t* list)
{
    tex_list_item_t* current = list->head;

    while (current)
    {
        tex_list_item_t* next = current->next;
        texture_t* tx = (texture_t*)current->tex;
        SDL_DestroyTexture(tx->texture);
        free(current);
        current = next;
    }
    free(current);
}