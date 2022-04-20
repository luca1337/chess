#include "utils.h"
#include "board.h"

#include <SDL.h>

int get_index_by_mouse_coords()
{
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    if (mouse_y >= SCREEN_H) return INVALID_INDEX;

    return ((mouse_y / BOARD_SZ) * CELLS_PER_ROW) + mouse_x / BOARD_SZ;
}