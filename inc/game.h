#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "player.h"
#include "queue.h"
#include "chess_piece.h"

typedef struct game{
    board_t* board;
    queue_t* players_queue;
    player_t* current_player;
    chess_piece_t* current_piece;
    piece_move_t* suggested_moves;
}game_t;

game_t* game_new();
void game_init(game_t* game);
void game_update(game_t* game);

#endif