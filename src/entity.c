#include "entity.h"
#include "private.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

entity_t* entity_new()
{
    entity_t* result = (entity_t*)calloc(1, sizeof(entity_t));
    CHECK(result, NULL, "Couldn't allocate memory for entity struct");
    return result;
}

void entity_destroy(entity_t* entity)
{
    free(entity);
}