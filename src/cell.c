#include "cell.h"
#include "private.h"
#include "texture.h"
#include "chess_piece.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void _draw_cell(cell_t *cell)
{
    cell->cell_texture->render(cell->cell_texture, 0, 0, NULL);
}

cell_t *cell_new(vec2_t pos, vec2_t sz, color_t draw_color)
{
    cell_t *cell = (cell_t *)calloc(1, sizeof(cell_t));
    CHECK(cell, NULL, "Couldn't allocate enought bytes for cell struct");

    cell->cell_texture = texture_create_raw(sz.xy[0], sz.xy[1], draw_color);
    cell->cell_texture->set_position(cell->cell_texture, pos.xy[0], pos.xy[1]);
    cell->draw = _draw_cell;
    cell->entity = NULL;

    return cell;
}

char is_cell_upper_bound(cell_t* cell)
{
    return cell != NULL && cell->pos_y == 0;
}

char is_cell_lower_bound(cell_t* cell)
{
    return cell != NULL && cell->pos_y == (SCREEN_H - CELL_SZ);
}

char is_cell_left_bound(cell_t* cell)
{
    return cell != NULL && cell->pos_x == 0;
}

char is_cell_right_bound(cell_t* cell)
{
    return cell != NULL && cell->pos_x == (SCREEN_W - CELL_SZ);
}

void cell_destroy(cell_t* cell)
{
    texture_destroy(cell->cell_texture);
}