#ifndef BOARD_H
#define BOARD_H

#include "texture.h"
#include "private.h"
#include "entity.h"
#include "vec2.h"

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

typedef struct cell{
    texture_t* cell_texture;
    entity_t* entity;
    char is_occupied;
    int pos_x, pos_y;
    void(*draw)(struct cell* cell);
}cell_t;

cell_t* cell_new(vec2_t pos, vec2_t sz, color_t);
char is_cell_busy(cell_t* cell);
char is_cell_upper_bound(cell_t* cell);
char is_cell_lower_bound(cell_t* cell);
char is_cell_left_bound(cell_t* cell);
char is_cell_right_bound(cell_t* cell);

typedef struct board{
    entity_t* entity;
    cell_t* cells[BOARD_SZ];
    void(*draw)(struct board* board);
}board_t;

board_t* board_new();

#endif