#ifndef BOARD_H
#define BOARD_H

#include "private.h"

typedef struct cell cell_t;

static const int board_matrix[BOARD_SZ] =
{
    ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK,
    PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
    ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK,
};

typedef struct board{
    cell_t* cells[BOARD_SZ];
    void(*draw)(struct board* board);
}board_t;

board_t* board_new();
void board_destroy(board_t* board);

#endif