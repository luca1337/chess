#ifndef ENTITY_H
#define ENTITY_H

#include "layer.h"

typedef struct entity{
    char enabled;
    layer_t z_layer;
    void(*render)(void** this);
}entity_t;

entity_t* entity_new();
void entity_destroy(entity_t* entity);

#endif