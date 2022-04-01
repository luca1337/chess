#ifndef CESS_PIECE_H
#define CESS_PIECE_H

#include "entity.h"
#include "utils.h"
#include "texture.h"
#include "board.h"
#include "player.h"

// which direction do we want to move on ?
typedef enum move_direction{
    east,
    north_east,
    north,
    north_west,
    west,
    south_west,
    south,
    south_east,
    // ---
    MAX_DIR
}move_direction_t;

// pawn moves
typedef struct piece_move{
    cell_t* possible_cells;
    texture_t* markers;
}piece_move_t;

piece_move_t* piece_move_new();

// base class that every piece will inherit from
typedef struct chess_piece{
    entity_t* entity; // do not use this!!
    piece_type_t piece_type;
    texture_t* chess_texture;
    piece_move_t** moves;
    int moves_number;
    char is_white;
    char is_killed;
    char is_first_move;
    int pos_x, pos_y;
    void(*draw)(struct chess_piece* piece);
    void(*set_position)(struct chess_piece* piece, int x, int y);
    piece_move_t**(*get_moves)(struct chess_piece* piece, board_t* board);
}chess_piece_t;

chess_piece_t* chess_piece_new(piece_type_t type, char is_white);
void piece_set_entity_cell(board_t* board, chess_piece_t* piece, int index);
void piece_set_entity_null(board_t* board, unsigned index);
char is_piece_near_upper_bound(chess_piece_t* piece);
char is_piece_near_lower_bound(chess_piece_t* piece);
char is_piece_near_left_bound(chess_piece_t* piece);
char is_piece_near_right_bound(chess_piece_t* piece);

#endif