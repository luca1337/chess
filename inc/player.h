#ifndef PLAYER_H
#define PLAYER_H

#include "chess_piece.h"

typedef struct player {
    char is_white;
    int score;
    const char* (*get_team)(struct player player);
    char has_promotion_pieces;
} player_t;

player_t* player_new(char is_white);
void player_destroy(player_t* player);

#endif