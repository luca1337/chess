#include "player.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

player_t* player_new(char is_white)
{
    player_t* player = (player_t*)malloc(sizeof(player_t));
    memset(player, 0, sizeof(player_t));
    player->is_white = is_white;
    return player;
}

void player_destroy(player_t* player)
{
    free(player->name);
    free(player);
}