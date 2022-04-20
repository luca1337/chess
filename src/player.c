#include "player.h"
#include "private.h"

#include <stdlib.h>

static const char* _get_team(struct player player) { return player.is_white ? "WHITE" : "BLACK"; }

player_t* player_new(char is_white)
{
    player_t* player = (player_t*)calloc(1, sizeof(player_t));
    CHECK(player, NULL, "Couldn't allocate memory for player struct");
    player->is_white = is_white;
    player->get_team = _get_team;
    return player;
}

void player_destroy(player_t* player) { free(player); }