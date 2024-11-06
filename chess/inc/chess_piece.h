#ifndef CESS_PIECE_H
#define CESS_PIECE_H

#include <private.h>
#include <queue.h>
#include <utils.h>

typedef struct board board_t;
typedef struct cell cell_t;
typedef struct texture texture_t;

// pawn moves
typedef struct piece_move {
    cell_t* possible_cells;
    texture_t* markers;
} piece_move_t;

piece_move_t* piece_move_new();
void piece_move_destroy(piece_move_t* move);

// move data
typedef struct chess_piece_move_data {
    int index_array[MAX_QUEUE_SIZE];
    int i, j;
} chess_piece_move_data_t;

// base piece data
typedef struct chess_piece_data {
    char is_white;
    char is_first_move;
    char is_enpassant;
    char is_castling;
    char has_eat_piece;
    char is_blocked;
} chess_piece_data_t;

typedef struct chess_piece {
    piece_type_t piece_type;
    texture_t* chess_texture;
    piece_move_t moves[MAX_QUEUE_SIZE];
    chess_piece_data_t piece_data;
    chess_piece_move_data_t possible_squares;
    chess_piece_move_data_t index_queue;
    unsigned long moves_number;
    int pos_x, pos_y;
    int score_value;
    int blocked_paths;
    void (*draw)(struct chess_piece* piece);
    void (*set_position)(struct chess_piece* piece, int x, int y);
    char (*generate_legal_moves)(struct chess_piece* piece, board_t* board, char simulate);
    char (*check_checkmate)(board_t* board, struct chess_piece* piece, cell_t* destination);
} chess_piece_t;

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