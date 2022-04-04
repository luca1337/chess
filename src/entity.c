#include "entity.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

entity_t* entity_new()
{
    entity_t* result = (entity_t*)malloc(sizeof(entity_t));
    memset(result, 0, sizeof(entity_t));
    return result;
}

void entity_destroy(entity_t* entity)
{
    free(entity);
}