#ifndef CELL_H
#define CELL_H

#include "color.h"
#include "vec2.h"


typedef struct texture texture_t;
typedef struct chess_piece chess_piece_t;

typedef struct cell {
    texture_t* cell_texture;
    chess_piece_t* entity;
    char is_occupied;
    int pos_x, pos_y;
    void (*draw)(struct cell* cell);
} cell_t;

cell_t* cell_new(vec2_t pos, vec2_t sz, color_t);
void cell_highlight(cell_t* cell, float x, float y, color_t);
char is_cell_busy(cell_t* cell);
char is_cell_upper_bound(cell_t* cell);
char is_cell_lower_bound(cell_t* cell);
char is_cell_left_bound(cell_t* cell);
char is_cell_right_bound(cell_t* cell);
void cell_restore_state(cell_t* cell);
void cell_destroy(cell_t* cell);

#endif