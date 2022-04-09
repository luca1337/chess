#ifndef CESS_PIECE_H
#define CESS_PIECE_H

#include "utils.h"
#include "queue.h"

typedef struct board board_t;
typedef struct cell cell_t;
typedef struct texture texture_t;

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
void piece_move_destroy(piece_move_t* move);

// base class that every piece will inherit from
typedef struct chess_piece{
    piece_type_t piece_type;
    texture_t* chess_texture;
    piece_move_t** moves;
    queue_t* index_queue;
    int* possible_squares;
    int moves_number;
    char is_white;
    char is_killed;
    char is_first_move;
    int pos_x, pos_y;
    void(*draw)(struct chess_piece* piece);
    void(*set_position)(struct chess_piece* piece, int x, int y);
    char(*generate_legal_moves)(struct chess_piece* piece, board_t* board);
    char(*check_checkmate)(board_t* board, struct chess_piece* piece, cell_t* destination);
}chess_piece_t;

// piece movements
char get_pawn_legal_moves(chess_piece_t* piece, board_t* board, char simulate);
char get_knight_legal_moves(chess_piece_t* piece, board_t* board, char simulate);
char get_queen_legal_moves(chess_piece_t* piece, board_t* board, char simulate);
char get_rook_legal_moves(chess_piece_t* piece, board_t* board, char simulate);
char get_bishop_legal_moves(chess_piece_t* piece, board_t* board, char simulate);
char get_king_legal_moves(chess_piece_t* piece, board_t* board, char simulate);

chess_piece_t* chess_piece_new(piece_type_t type, char is_white, const char use_blending);
void chess_piece_set_entity_cell(board_t* board, chess_piece_t* piece, int index);
void chess_piece_set_entity_null(board_t* board, unsigned index);
char chess_piece_is_near_upper_bound(chess_piece_t* piece);
char chess_piece_is_near_lower_bound(chess_piece_t* piece);
char chess_piece_is_near_left_bound(chess_piece_t* piece);
char chess_piece_is_near_right_bound(chess_piece_t* piece);
void chess_piece_destroy(chess_piece_t* piece);
const char* chess_piece_to_string(chess_piece_t* piece);

#endif