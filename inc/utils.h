#ifndef UTILS_H
#define UTILS_H

typedef struct board board_t;

typedef enum piece_type{
    none = 0,
    rook,
    knight,
    bishop,
    queen,
    king,
    pawn
} piece_type_t;

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

int get_index_by_mouse_coords();

#endif