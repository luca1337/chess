#include <cell.h>
#include <chess_piece.h>
#include <private.h>
#include <texture.h>

#include <stdlib.h>
#include <string.h>

cell_t* previous_cell = NULL;

static void _draw_cell(cell_t* cell)
{
    if (!cell) return;

    cell->cell_texture->render(cell->cell_texture, 0, NULL);
}

cell_t* cell_new(vec2 pos, vec2 sz, color_t draw_color)
{
    cell_t* cell = (cell_t*)calloc(1, sizeof(cell_t));
    CHECK(cell, NULL, "Couldn't allocate enought bytes for cell struct");

    cell->cell_texture = texture_create_raw((uint32_t)sz[0], (uint32_t)sz[1], draw_color);
    cell->cell_texture->set_position(cell->cell_texture, (int)pos[0], (int)pos[1]);
    cell->draw = _draw_cell;
    cell->entity = NULL;

    return cell;
}

void cell_highlight(cell_t* cell, float mouse_x, float mouse_y, color_t color)
{
    SDL_assert_always(cell);

    cell_t* current_cell = cell;

    if (!current_cell) return;

    if ((mouse_x > current_cell->pos_x && mouse_x < (current_cell->pos_x + CELL_SZ)) && (mouse_y > current_cell->pos_y && (mouse_y < current_cell->pos_y + CELL_SZ)) && !previous_cell)
    {
        SDL_SetTextureColorMod(cell->cell_texture->texture, color.r, color.g, color.b);
        previous_cell = current_cell;
    }

    if (previous_cell && (memcmp(current_cell, previous_cell, sizeof(cell_t))))
    {
        // restore color of previous cell before selecting the new one
        SDL_SetTextureColorMod(previous_cell->cell_texture->texture, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX);
        previous_cell = NULL;
    }
}

char is_cell_upper_bound(cell_t* cell) { return cell != NULL && cell->pos_y == 0; }

char is_cell_lower_bound(cell_t* cell) { return cell != NULL && cell->pos_y == (SCREEN_H - CELL_SZ); }

char is_cell_left_bound(cell_t* cell) { return cell != NULL && cell->pos_x == 0; }

char is_cell_right_bound(cell_t* cell) { return cell != NULL && cell->pos_x == (SCREEN_W - CELL_SZ); }

void cell_destroy(cell_t* cell) { texture_destroy(cell->cell_texture); }

void cell_restore_state(cell_t* cell) { cell->is_occupied = FALSE; }