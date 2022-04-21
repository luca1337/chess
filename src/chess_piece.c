#include "chess_piece.h"
#include "board.h"
#include "cell.h"
#include "events.h"
#include "game.h"
#include "player.h"
#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sglib.h>

extern events_t *events;
extern texture_pool_t texture_pool;

const char *white_png_postfix = "_w.comp";
const char *black_png_postfix = "_b.comp";

unsigned depth = 0;

const char *assets[7] = {
    "",
    "../assets/textures/rook",
    "../assets/textures/knight",
    "../assets/textures/bishop",
    "../assets/textures/queen",
    "../assets/textures/king",
    "../assets/textures/pawn",
};

static texture_t *get_chess_texture(piece_type_t type, char is_white, const char use_blending)
{
    texture_t *result = NULL;

    const char *value = assets[(int)type];

    int prefix_len = strlen(value);
    int post_fix_len = strlen(white_png_postfix);
    int total_buf_len = sizeof(char) * (prefix_len + post_fix_len) + 1;

    // need another buffer to store the concatenation result
    char *path = (char *)malloc(total_buf_len);
    strcpy_s(path, total_buf_len, value);
    strcat_s(path, total_buf_len, is_white ? white_png_postfix : black_png_postfix);
    // path[total_buf_len] = '\0';

    result = texture_load_from_file(path, use_blending);

    free(path);

    return result;
}

static void _draw_piece(struct chess_piece *piece) { piece->chess_texture->render(piece->chess_texture, 0, NULL); }

static void _set_position(struct chess_piece *piece, int x, int y)
{
    piece->pos_x = x;
    piece->pos_y = y;
    piece->chess_texture->set_position(piece->chess_texture, x, y);
}

piece_move_t *piece_move_new()
{
    piece_move_t *move = (piece_move_t *)calloc(1, sizeof(piece_move_t));
    move->markers = &SGLIB_QUEUE_FIRST_ELEMENT(texture_t, texture_pool.textures, texture_pool.i, texture_pool.j); // get texture from queue without allocating anything
    SGLIB_QUEUE_DELETE_FIRST(texture_t *, texture_pool.textures, texture_pool.i, texture_pool.j, TEXTURE_POOL_SIZE);
    return move;
}

void piece_move_destroy(piece_move_t *move)
{
    cell_destroy(move->possible_cells);
    texture_destroy(move->markers);
    free(move);
}

static int get_cell_index_by_piece_position(chess_piece_t *piece, int x_offset, int y_offset) { return (((piece->pos_y / CELL_SZ) * CELLS_PER_ROW) + (piece->pos_x / CELL_SZ) + x_offset) + (y_offset * CELLS_PER_ROW); }

static char is_cell_occupied_by_friendly_piece(cell_t *cell, chess_piece_t *piece) { return (cell->is_occupied && cell->entity->piece_data.is_white == piece->piece_data.is_white); }

static char is_cell_occupied_by_enemy_piece(cell_t *cell, chess_piece_t *piece) { return (cell->is_occupied && cell->entity->piece_data.is_white != piece->piece_data.is_white); }

static char is_cell_walkable_by_piece(cell_t *cell, chess_piece_t *piece) { return (is_cell_occupied_by_enemy_piece(cell, piece) || !cell->is_occupied); }

static char allocate_legal_moves(chess_piece_t *piece, board_t *board, char simulate)
{
    // if no moves are available just go out and this pawn cannot move.
    if (piece->moves_number == 0) { return FALSE; }

    // We distinguish the simulation because, in this case, we only care abouts "indexes" of the board
    // that the pawn could go if moved. This piece of code is called only when the king is selected and
    // in this way we can start "simulating" all the possible moves that all the enemy pieces could do
    // in the next turn, to se whether the king will be in checkmate.
    if (simulate)
    {
        SGLIB_QUEUE_INIT(int, piece->possible_squares.index_array, piece->possible_squares.i, piece->possible_squares.j);

        for (unsigned long i = 0ul; i < piece->moves_number; ++i)
        {
            const int index = SGLIB_QUEUE_FIRST_ELEMENT(int, piece->index_queue.index_array, piece->index_queue.i, piece->index_queue.j);
            SGLIB_QUEUE_ADD(int, piece->possible_squares.index_array, index, piece->possible_squares.i, piece->possible_squares.j, MAX_QUEUE_SIZE);
            SGLIB_QUEUE_DELETE(int, piece->index_queue.index_array, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
        }
    } else
    {
        // TODO: implement memory pool also here. Again, allocating each time this piece of memory is not correct at all.
        piece->moves = (piece_move_t **)calloc(piece->moves_number, sizeof(piece_move_t *));
        CHECK(piece->moves, NULL, "Couldn't allocate memory for piece->moves ");

        for (unsigned long i = 0ul; i < piece->moves_number; ++i)
        {
            const int index = SGLIB_QUEUE_FIRST_ELEMENT(int, piece->index_queue.index_array, piece->index_queue.i, piece->index_queue.j);

            piece->moves[i] = piece_move_new();
            piece->moves[i]->possible_cells = board->cells[index];
            piece->moves[i]->markers->set_position(piece->moves[i]->markers, piece->moves[i]->possible_cells->pos_x, piece->moves[i]->possible_cells->pos_y);

            SGLIB_QUEUE_DELETE(int, piece->index_queue.index_array, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
        }
    }

    return TRUE;
}

char get_pawn_legal_moves(chess_piece_t *piece, board_t *board, char simulate)
{
    // reset variables
    piece->moves_number = 0;

    const char is_first_move = piece->piece_data.is_first_move;
    const char is_white = piece->piece_data.is_white;

    // matrice in indici verticali
    const int vertical_moves_count = is_first_move ? 2 : 1;
    const int diagonal_moves_count = 2;
    const int enpassant_moves_count = 2;

    // se è la prima volta che muovo la pedina, posso suggerire due caselle in verticale ( solo x muovermi e non x mangiare )
    // invece le caselle in diagonale possono sempre essere suggerite in quanto servono solo per mangiare e non per spostarsi liberamente
    SGLIB_QUEUE_INIT(int, piece->index_queue.index_array, piece->index_queue.i, piece->index_queue.j);

    // vertical squares
    const int first_vert_idx = is_white ? get_cell_index_by_piece_position(piece, -0, -1) : get_cell_index_by_piece_position(piece, +0, +1);
    const int second_vert_idx = is_white ? get_cell_index_by_piece_position(piece, -0, -2) : get_cell_index_by_piece_position(piece, +0, +2);

    // diagonal squares
    const int diagonal_left_idx = is_white ? get_cell_index_by_piece_position(piece, -1, -1) : get_cell_index_by_piece_position(piece, -1, +1);
    const int diagonal_right_idx = is_white ? get_cell_index_by_piece_position(piece, +1, -1) : get_cell_index_by_piece_position(piece, +1, +1);

    // e.p indexes
    const int enpassant_left_idx = get_cell_index_by_piece_position(piece, -1, -0);
    const int enpassant_right_idx = get_cell_index_by_piece_position(piece, +1, +0);

    const int possible_squares[] = {
        enpassant_left_idx,
        enpassant_right_idx,
        diagonal_left_idx,
        diagonal_right_idx,
        first_vert_idx,
        second_vert_idx // this index it's basically checked only once (when the pawn is moved for the first time, since it can move by 2 squares)
    };

    // verticale e diagonale
    for (unsigned long squareIdx = 0ul; squareIdx != (enpassant_moves_count + diagonal_moves_count + vertical_moves_count); ++squareIdx)
    {
        int matrix_index = possible_squares[squareIdx];

        if (matrix_index < 0 || matrix_index > (BOARD_SZ - 1)) continue;

        cell_t *current_cell = board->cells[matrix_index];

        if (squareIdx < (enpassant_moves_count + diagonal_moves_count))
        {
            // Just check whether we should skip the check based on oddity/evenly
            const char skip_lateral_bounds = (!(squareIdx % 2)) ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

            if (skip_lateral_bounds || !current_cell) { continue; }

            // Handle enpassant
            if (squareIdx < enpassant_moves_count)
            {
                // arrivati qui siamo sicuri che la cella digonale NON sia nulla
                const chess_piece_t *current_cell_piece = current_cell->entity;

                if (current_cell_piece && (current_cell_piece->piece_data.is_enpassant && current_cell_piece->piece_data.is_white != is_white))
                {
                    matrix_index = is_white ? matrix_index - CELLS_PER_ROW : matrix_index + CELLS_PER_ROW;

                    SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, matrix_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                    piece->moves_number++;
                    continue;
                } else
                {
                    continue;
                }
            } else // Handle diagonal movements
            {
                const chess_piece_t *current_cell_piece = current_cell->entity;

                // we cannot move
                if ((current_cell_piece && (current_cell_piece->piece_data.is_white == is_white)) || !current_cell_piece) continue;
            }
        } else
        {
            if (current_cell->is_occupied) { break; }
        }

        SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, matrix_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
        piece->moves_number++;
    }

    return allocate_legal_moves(piece, board, simulate);
}

char get_knight_legal_moves(chess_piece_t *piece, board_t *board, char simulate)
{
    // the knight can move in a "L" shape in all directions

    // reset variables
    piece->moves_number = 0;

    SGLIB_QUEUE_INIT(int, piece->index_queue.index_array, piece->index_queue.i, piece->index_queue.j);

    // we precalculate all the possible indexes that the piece could move on
    // and since the knight can jump over the piece, we don't actually care to
    // check the neighbors' cell in that direction.

    // up left, left up
    const int left_up_idx = get_cell_index_by_piece_position(piece, -2, -1);
    const int up_left_idx = get_cell_index_by_piece_position(piece, -1, -2);

    // up right, right up
    const int up_right_idx = get_cell_index_by_piece_position(piece, +1, -2);
    const int right_up_idx = get_cell_index_by_piece_position(piece, +2, -1);

    // right down, down right
    const int right_down_idx = get_cell_index_by_piece_position(piece, +2, +1);
    const int down_right_idx = get_cell_index_by_piece_position(piece, +1, +2);

    // down left, left down
    const int down_left_idx = get_cell_index_by_piece_position(piece, -1, +2);
    const int left_down_idx = get_cell_index_by_piece_position(piece, -2, +1);

    // Knight boundaries
    const char is_knight_within_left_bound = piece->pos_x >= 128;
    const char is_knight_within_right_bound = piece->pos_x <= 320;

    // Start checking all the 8 directions!
    move_direction_t move_direction = east;
    while (move_direction != MAX_DIR)
    {
        for (ever)
        {
            int possible_square = 0;

            // Check direction by direction in a clock-wise order.

            switch (move_direction)
            {
            default: break;
            case east: possible_square = left_up_idx; break;
            case north_east: possible_square = up_left_idx; break;
            case north: possible_square = up_right_idx; break;
            case north_west: possible_square = right_up_idx; break;
            case west: possible_square = right_down_idx; break;
            case south_west: possible_square = down_right_idx; break;
            case south: possible_square = down_left_idx; break;
            case south_east: possible_square = left_down_idx; break;
            }

            if (possible_square < 0 || possible_square > (BOARD_SZ - 1)) break;

            const char is_east = move_direction == east;
            const char is_north_east = move_direction == north_east;
            const char is_south = move_direction == south;
            const char is_south_east = move_direction == south_east;
            const char is_north = move_direction == north;
            const char is_north_west = move_direction == north_west;
            const char is_west = move_direction == west;
            const char is_south_west = move_direction == south_west;

            // Here we check the boundaries making sure that the knight it's always
            // inside the 8x8 matrix while checking it's possible moves

            if (is_south && !(piece->pos_x >= 64)) break;
            if (is_north && !(piece->pos_x <= 384)) break;
            if ((is_east || is_north_east || is_south_east) && !is_knight_within_left_bound) break;
            if ((is_north_west || is_west || is_south_west) && !is_knight_within_right_bound) break;

            cell_t *current_cell = board->cells[possible_square];

            if (is_cell_occupied_by_friendly_piece(current_cell, piece)) break;

            if (is_cell_walkable_by_piece(current_cell, piece))
            {
                // Enqueue the cell index only if this condition is satisfied, that means, this index will be part of legal moves
                SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, possible_square, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                piece->moves_number++;
                break;
            }
        }

        move_direction++;
    }

    return allocate_legal_moves(piece, board, simulate);
}

char get_queen_legal_moves(chess_piece_t *piece, board_t *board, char simulate)
{
    piece->moves_number = 0;

    SGLIB_QUEUE_INIT(int, piece->index_queue.index_array, piece->index_queue.i, piece->index_queue.j);

    // indice corrente della pedina nella matrice 8x8
    int piece_index = get_cell_index_by_piece_position(piece, 0, 0);

    // the queen can move along all the six direction as far as possible until she encounter her ally or an enemy
    // we must check every possible direction starting from the nearest one
    // This algorithm can be simplified a lot, but i'm too lazy now let's leave it like it is.

    // turning from east to sout-east all aorund in a clock-wise order
    move_direction_t move_direction = east;

    int step = 1;
    while (move_direction != MAX_DIR)
    {
        int matrix_index = 0;
        for (ever)
        {
            if (move_direction == east || move_direction == west)
            {
                char is_east = move_direction == east;
                char is_near_lateral_bounds = is_east ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

                if (is_near_lateral_bounds) break;

                matrix_index = is_east ? (piece_index - step) : (piece_index + step);

                cell_t *current_cell = board->cells[matrix_index];
                char is_cell_near_lateral_bounds = is_east ? is_cell_left_bound(current_cell) : is_cell_right_bound(current_cell);

                if (is_cell_occupied_by_friendly_piece(current_cell, piece)) break;

                if (is_cell_near_lateral_bounds && is_cell_walkable_by_piece(current_cell, piece))
                {
                    SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, matrix_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                    piece->moves_number++;
                    break;
                }

                if (is_cell_occupied_by_enemy_piece(current_cell, piece))
                {
                    SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, matrix_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                    piece->moves_number++;
                    break;
                }

                SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, matrix_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);

                step++;
                piece->moves_number++;
            } else if (move_direction == north_east || move_direction == north_west)
            {
                char is_north_east = move_direction == north_east;
                char is_near_lateral_bounds = is_north_east ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

                matrix_index = is_north_east ? (piece_index - 1) - CELLS_PER_ROW : (piece_index + 1) - CELLS_PER_ROW;

                if (is_near_lateral_bounds || matrix_index < 0) break;

                cell_t *current_cell = board->cells[matrix_index];
                char is_cell_near_lateral_bounds = is_north_east ? is_cell_left_bound(current_cell) : is_cell_right_bound(current_cell);

                if (is_cell_occupied_by_friendly_piece(current_cell, piece)) break;

                if (is_cell_near_lateral_bounds && is_cell_walkable_by_piece(current_cell, piece))
                {
                    SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, matrix_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                    piece->moves_number++;
                    break;
                }

                if (is_cell_occupied_by_enemy_piece(current_cell, piece))
                {
                    SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, matrix_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                    piece->moves_number++;
                    break;
                }

                SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, matrix_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);

                piece_index = matrix_index;

                step++;
                piece->moves_number++;
            } else if (move_direction == north || move_direction == south)
            {
                char is_north = move_direction == north;

                matrix_index = is_north ? (piece_index - (CELLS_PER_ROW * step)) : (piece_index + (CELLS_PER_ROW * step));

                if (matrix_index < 0 || matrix_index > (BOARD_SZ - 1)) break;

                cell_t *current_cell = board->cells[matrix_index];

                if (is_cell_occupied_by_friendly_piece(current_cell, piece)) break;

                if (is_cell_occupied_by_enemy_piece(current_cell, piece))
                {
                    piece->moves_number++;
                    SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, matrix_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                    break;
                }

                SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, matrix_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);

                step++;
                piece->moves_number++;
            } else if (move_direction == south_west || move_direction == south_east)
            {
                char is_south_west = move_direction == south_west;
                char is_queen_near_lateral_bounds = is_south_west ? chess_piece_is_near_right_bound(piece) : chess_piece_is_near_left_bound(piece);

                matrix_index = is_south_west ? (piece_index + 1) + CELLS_PER_ROW : (piece_index - 1) + CELLS_PER_ROW;

                if (is_queen_near_lateral_bounds || matrix_index > (BOARD_SZ - 1)) break;

                cell_t *current_cell = board->cells[matrix_index];

                char is_cell_near_lateral_bounds = is_south_west ? is_cell_right_bound(current_cell) : is_cell_left_bound(current_cell);

                if (is_cell_occupied_by_friendly_piece(current_cell, piece)) break;

                if (is_cell_near_lateral_bounds && is_cell_walkable_by_piece(current_cell, piece))
                {
                    SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, matrix_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                    piece->moves_number++;
                    break;
                }

                if (is_cell_occupied_by_enemy_piece(current_cell, piece))
                {
                    SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, matrix_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                    piece->moves_number++;
                    break;
                }

                SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, matrix_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);

                piece_index = matrix_index;

                piece->moves_number++;
                step++;
            } else
                break;
        }

        piece_index = get_cell_index_by_piece_position(piece, 0, 0);
        step = 1;
        move_direction++;
    }

    return allocate_legal_moves(piece, board, simulate);
}

char get_rook_legal_moves(chess_piece_t *piece, board_t *board, char simulate)
{
    piece->moves_number = 0;

    SGLIB_QUEUE_INIT(int, piece->index_queue.index_array, piece->index_queue.i, piece->index_queue.j);

    // indice corrente della pedina nella matrice 8x8
    int piece_index = get_cell_index_by_piece_position(piece, 0, 0);

    // ROOK can vertically and horizontally on the board, so here we only check those directions
    // East && West are pretty the same, just the index will be swapped as well as North && South
    // Basically we take the selected chess piece as reference and increment/decrement it's current index on the board
    // based on the direction we want to look for.

    // all the possible directions for the rook, so we will look for each direction, until it will find an obstacle
    move_direction_t move_directions[4] = {east, north, west, south};

    // start step from 1 so that we can already look in the adjacent cell in the first iteration and also we don't multiply by 0
    int step = 1;
    for (unsigned long dirIdx = 0ul; dirIdx != _countof(move_directions); ++dirIdx)
    {
        cell_t *current_cell = NULL;
        chess_piece_t *current_cell_piece = NULL;

        move_direction_t current_dir = move_directions[dirIdx];
        int cell_index = 0;
        for (ever)
        {
            if (current_dir == east || current_dir == west)
            {
                char is_east = current_dir == east;
                char is_near_lateral_bounds = is_east ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

                if (is_near_lateral_bounds) break;

                cell_index = is_east ? (piece_index - step) : (piece_index + step);

                current_cell = board->cells[cell_index];
                current_cell_piece = current_cell->entity;

                char is_current_cell_near_bounds = is_east ? is_cell_left_bound(current_cell) : is_cell_right_bound(current_cell);

                if (is_current_cell_near_bounds && ((current_cell->is_occupied && current_cell_piece->piece_data.is_white != piece->piece_data.is_white) || !current_cell->is_occupied))
                {
                    SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, cell_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                    piece->moves_number++;
                    break;
                }
            } else
            {
                char is_north = current_dir == north;

                cell_index = is_north ? (piece_index - (CELLS_PER_ROW * step)) : (piece_index + (CELLS_PER_ROW * step));

                if (cell_index < 0 || cell_index > (BOARD_SZ - 1)) break;

                current_cell = board->cells[cell_index];
                current_cell_piece = current_cell->entity;
            }

            if (current_cell->is_occupied && current_cell_piece->piece_data.is_white != piece->piece_data.is_white)
            {
                SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, cell_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                piece->moves_number++;
                break;
            }

            if ((current_cell->is_occupied && current_cell_piece->piece_data.is_white == piece->piece_data.is_white)) break;

            SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, cell_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);

            step++;
            piece->moves_number++;
        }

        // reset back the piece index because while checking directions, we increment the index to look
        // and in the end we bring it back (where the chess piece is located)
        piece_index = get_cell_index_by_piece_position(piece, 0, 0);
        step = 1;
    }

    return allocate_legal_moves(piece, board, simulate);
}

char get_bishop_legal_moves(chess_piece_t *piece, board_t *board, char simulate)
{
    piece->moves_number = 0;

    SGLIB_QUEUE_INIT(int, piece->index_queue.index_array, piece->index_queue.i, piece->index_queue.j);

    int piece_index = get_cell_index_by_piece_position(piece, 0, 0);

    // We move in 4 diagonal directions
    const move_direction_t move_directions[4] = {north_east, north_west, south_east, south_west};

    int step = 1;
    for (unsigned long dirIdx = 0ul; dirIdx != _countof(move_directions); ++dirIdx)
    {
        move_direction_t current_dir = move_directions[dirIdx];
        int cell_index = 0;
        for (ever)
        {
            const char is_south_west = current_dir == south_west;
            const char is_north_west = current_dir == north_west;
            const char is_north_east = current_dir == north_east;

#pragma region CALCULATE_CORRECT_BOARD_INDEX
            if (is_north_east || is_north_west)
            {
                const char is_near_lateral_bounds = is_north_east ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

                cell_index = is_north_east ? (piece_index - 1) - CELLS_PER_ROW : (piece_index + 1) - CELLS_PER_ROW;

                if (is_near_lateral_bounds || cell_index < 0) break;
            } else
            {
                const char is_bishop_near_lateral_bounds = is_south_west ? chess_piece_is_near_right_bound(piece) : chess_piece_is_near_left_bound(piece);

                cell_index = is_south_west ? (piece_index + 1) + CELLS_PER_ROW : (piece_index - 1) + CELLS_PER_ROW;

                if (is_bishop_near_lateral_bounds || cell_index > (BOARD_SZ - 1)) break;
            }
#pragma endregion

#pragma region CHECK_IF_CELL_IS AVAILABLE
            cell_t *current_cell = board->cells[cell_index];

            int north_east_west_index = is_north_east ? is_cell_left_bound(current_cell) : is_cell_right_bound(current_cell);
            int south_east_west_index = is_south_west ? is_cell_right_bound(current_cell) : is_cell_left_bound(current_cell);
            char is_cell_near_lateral_bounds = (is_north_east || is_north_west) ? north_east_west_index : south_east_west_index;

            // Break when the cell is occupied by friendly piece
            if (is_cell_occupied_by_friendly_piece(current_cell, piece)) break;

            if (is_cell_near_lateral_bounds && is_cell_walkable_by_piece(current_cell, piece))
            {
                SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, cell_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                piece->moves_number++;
                break;
            }

            if (is_cell_occupied_by_enemy_piece(current_cell, piece))
            {
                SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, cell_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                piece->moves_number++;
                break;
            }

            SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, cell_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);

            piece_index = cell_index;
            step++;
            piece->moves_number++;
        }
#pragma endregion

        piece_index = get_cell_index_by_piece_position(piece, 0, 0);
        step = 1;
    }

    return allocate_legal_moves(piece, board, simulate);
}

static char check_king_rescue(chess_piece_t *piece, board_t *board, cell_t *destination)
{
    for (unsigned long cellIdx = 0ul; cellIdx != BOARD_SZ; ++cellIdx)
    {
        chess_piece_t *friedly_piece = board->cells[cellIdx]->entity;

        // When the found piece it's a king (ourselves) ofc we must skip
        if (friedly_piece && (piece->piece_data.is_white == friedly_piece->piece_data.is_white && friedly_piece->piece_type == king)) { continue; }

        if (!friedly_piece || (friedly_piece && ((piece->piece_data.is_white != friedly_piece->piece_data.is_white)))) continue;

        if (piece->check_checkmate(board, friedly_piece, destination))
        {
            piece->blocked_paths--;
            piece->piece_data.is_blocked = FALSE;
            return TRUE;
        }
    }

    return FALSE;
}

static char can_reach_cell(chess_piece_t *piece_to_save, chess_piece_t *enemy_piece, board_t *board, cell_t *destination)
{
    if (!piece_to_save->check_checkmate(board, enemy_piece, destination))
    {
        piece_to_save->blocked_paths--;
        piece_to_save->piece_data.is_blocked = FALSE;
        return FALSE;
    }

    return TRUE;
}

static char check_if_remaining_pieces_can_move(chess_piece_t *piece_family, board_t *board)
{
    for (unsigned long i = 0; i != BOARD_SZ; ++i)
    {
        chess_piece_t *piece = board->cells[i]->entity;

        if (!piece) continue;

        if (piece->piece_data.is_white != piece_family->piece_data.is_white) continue;

        if (piece->piece_data.is_white == piece_family->piece_data.is_white && piece->piece_type == king) { continue; }

        if (piece->generate_legal_moves(piece, board, TRUE)) { return TRUE; }
    }

    return FALSE;
}

char get_king_legal_moves(chess_piece_t *piece, board_t *board, char simulate)
{
    // the king can move to adjacent cells in all directions, by one square.
    char is_castling_blocked = FALSE;
    piece->moves_number = 0;

    char can_castle = 0;

    SGLIB_QUEUE_INIT(int, piece->index_queue.index_array, piece->index_queue.i, piece->index_queue.j);

    // precalculate possible squares
    const int lefx_idx = get_cell_index_by_piece_position(piece, -1, -0);
    const int up_left_idx = get_cell_index_by_piece_position(piece, -1, -1);
    const int up_idx = get_cell_index_by_piece_position(piece, -0, -1);
    const int right_idx = get_cell_index_by_piece_position(piece, +1, +0);
    const int up_right_idx = get_cell_index_by_piece_position(piece, +1, -1);
    const int down_right_idx = get_cell_index_by_piece_position(piece, +1, +1);
    const int down_idx = get_cell_index_by_piece_position(piece, +0, +1);
    const int down_left_idx = get_cell_index_by_piece_position(piece, -1, +1);

    int possible_square = 0;
    move_direction_t move_direction = east;
    while (move_direction != MAX_DIR)
    {
        const char is_east = move_direction == east;
        const char is_north_east = move_direction == north_east;
        const char is_south_east = move_direction == south_east;
        const char is_north_west = move_direction == north_west;
        const char is_west = move_direction == west;
        const char is_south_west = move_direction == south_west;

        const char check_left_castling = (is_east && piece->piece_data.is_first_move);
        const char check_right_castling = (is_west && piece->piece_data.is_first_move);

        // Left castling is is 4 steps further but we start from 1 so we count until 5
        // same for the right castling (short) is 3 steps but we count until 4
        const int lest_castling_max_steps = 5, right_castling_max_steps = 4, max_king_steps_per_direction = 2;
        const int max_steps = check_left_castling ? lest_castling_max_steps : check_right_castling ? right_castling_max_steps : max_king_steps_per_direction;

        for (unsigned long step = 1; step != max_steps; step++)
        {
            switch (move_direction)
            {
            default: break;
            case east: possible_square = simulate ? lefx_idx : get_cell_index_by_piece_position(piece, -step, -0); break; // castling on east (long castling)
            case north_east: possible_square = up_left_idx; break;
            case north: possible_square = up_idx; break;
            case north_west: possible_square = up_right_idx; break;
            case west: possible_square = simulate ? right_idx : get_cell_index_by_piece_position(piece, +step, +0); break;
            case south_west: possible_square = down_right_idx; break;
            case south: possible_square = down_idx; break;
            case south_east: possible_square = down_left_idx; break;
            }

            if (possible_square < 0 || possible_square > (BOARD_SZ - 1)) break;

            if ((is_east || is_north_east || is_south_east) && chess_piece_is_near_left_bound(piece)) break;

            if ((is_north_west || is_west || is_south_west) && chess_piece_is_near_right_bound(piece)) break;

            cell_t *const current_cell = board->cells[possible_square];

            // check checkmate: to implement a check checkmate algorithm, we need to check for each piece present on the board
            // if it can occupy one of the hypotetical available cells for the king

            if (is_cell_occupied_by_friendly_piece(current_cell, piece))
            {
                // if we arrived on the last step and the rook is present, king can castle.
                if (((step == 4 && check_left_castling) || (step == 3 && check_right_castling)) && (current_cell->entity && current_cell->entity->piece_type == rook))
                {
                    can_castle = TRUE;
                    break;
                }

                is_castling_blocked = TRUE;
                break;
            }

            if (!current_cell->is_occupied || is_cell_occupied_by_enemy_piece(current_cell, piece))
            {
                // when the piece currently checked it's a king then this function will be called recursively but we must avoid that
                if (!simulate && depth < 1)
                {
                    // only here the king could move but we must check if that cell will turn him into checkmate
                    // we must iterate over all the enemies' pawns and check whether they could reach this cell
                    cell_t *blocking_cell = NULL;
                    for (unsigned long cellIdx = 0ul; cellIdx != BOARD_SZ; ++cellIdx)
                    {
                        chess_piece_t *enemy_pc = board->cells[cellIdx]->entity;

                        if (enemy_pc && (piece->piece_data.is_white != enemy_pc->piece_data.is_white && enemy_pc->piece_type == king)) { depth++; }

                        if (!enemy_pc || (enemy_pc && ((piece->piece_data.is_white == enemy_pc->piece_data.is_white)))) continue;

                        // we stop everytime a enemy piece can move in the same cell that the king could move to
                        if (piece->check_checkmate(board, enemy_pc, current_cell))
                        {
                            // if we arrive here it doesn't necessarily means that the king is in checkmate
                            // given that we have last some pawn we should check if some of them could kill the
                            // enemy pawn that is threatening the king. if noone can kill that enemy then, the king
                            // totally in checkmate and the game is lost for that team.
                            piece->blocked_paths++;
                            piece->piece_data.is_blocked = TRUE;
                            is_castling_blocked = TRUE;
                            blocking_cell = board->cells[cellIdx];
                            break;
                        }
                    }

                    if (piece->piece_data.is_blocked)
                    {
                        // the king is blocked, try to rescue him by checking if any of friendly pawn can kill the blocking enemy.
                        // if a pawn can rescue the king it means that he will not have any available move so he's blocked but the game is not ended yet

                        if (!check_king_rescue(piece, board, current_cell))
                        {
                            // Check if any of remaining friendly chess pieces can rescue the king by reaching the blocking enemy cell
                            if (!check_king_rescue(piece, board, blocking_cell))
                            {
                                // Last check: check if the blocking enemy will not reach the king we are safe!
                                if (!can_reach_cell(piece, blocking_cell->entity, board, board->cells[get_cell_index_by_piece_position(piece, 0, 0)])) { goto exit; }

                                piece->blocked_paths++;
                            }
                        }
                        break;
                    }
                }

                if (step == 1)
                {
                    SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, possible_square, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
                    piece->moves_number++;
                    piece->piece_data.is_blocked = FALSE;
                }
            }
        }

        if (can_castle && !simulate)
        {
            int castle_index = is_east ? get_cell_index_by_piece_position(piece, -2, -0) : get_cell_index_by_piece_position(piece, +2, +0);

            SGLIB_QUEUE_ADD(int, piece->index_queue.index_array, castle_index, piece->index_queue.i, piece->index_queue.j, MAX_QUEUE_SIZE);
            piece->moves_number++;
        }
    exit:

        move_direction++;
        piece->piece_data.is_blocked = FALSE;
        can_castle = FALSE;
    }

    depth = 0;

    // In this case the king will be totally blocked for now and the game is ended
    char can_piece_still_move = check_if_remaining_pieces_can_move(piece, board);
    if (piece->blocked_paths > 0 && piece->moves_number == 0 && !can_piece_still_move) { piece->piece_data.is_blocked = TRUE; }

    return allocate_legal_moves(piece, board, simulate);
}

char _generate_legal_moves(struct chess_piece *piece, board_t *board, char simulate)
{
    // Here we just generate legal moves for each known type we picked up with mouse

    switch (piece->piece_type)
    {
    default: break;
    case rook: return get_rook_legal_moves(piece, board, simulate);
    case knight: return get_knight_legal_moves(piece, board, simulate);
    case bishop: return get_bishop_legal_moves(piece, board, simulate);
    case queen: return get_queen_legal_moves(piece, board, simulate);
    case king: return get_king_legal_moves(piece, board, simulate);
    case pawn: return get_pawn_legal_moves(piece, board, simulate);
    }

    return FALSE;
}

char _check_checkmate(board_t *board, struct chess_piece *piece, cell_t *destination)
{
    // the checkmate algorithm it's nothing more than a simulation of the legal move's one
    // for every enemy piece that could potentially reach any of the king's cell, so we call
    // get_piece_legal_moves it's done.
    const char should_simulate = TRUE;
    const piece_type_t piece_type = piece->piece_type;

    const char *player_color = piece->piece_data.is_white ? "WHITE" : "BLACK";

    char result = FALSE;

    // this pawn check can also be done by extending the original get_pawn_moves (like all the others) to avoid code duplication but for now let's leave it like this.
    if (piece_type == pawn)
    {
        const int possible_moves_count = 2;

        const int diagonal_left_idx = piece->piece_data.is_white ? get_cell_index_by_piece_position(piece, -1, -1) : get_cell_index_by_piece_position(piece, -1, +1);
        const int diagonal_right_idx = piece->piece_data.is_white ? get_cell_index_by_piece_position(piece, +1, -1) : get_cell_index_by_piece_position(piece, +1, +1);

        const int possible_moves[] = {diagonal_left_idx, diagonal_right_idx};

        for (unsigned long moveIdx = 0ul; moveIdx != possible_moves_count; ++moveIdx)
        {
            int index_to_look = possible_moves[moveIdx];
            char skip_lateral_bounds = moveIdx == 0 ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

            if (index_to_look < 0 || index_to_look > (BOARD_SZ - 1) || skip_lateral_bounds) continue;

            const cell_t *const cell_to_look = board->cells[index_to_look];

            if (!SDL_memcmp(cell_to_look, destination, sizeof(cell_t)))
            {
                SDL_Log("[[CHECK CHECKMATE]]: [%s] [%s] can move on index: [%i]", player_color, chess_piece_to_string(piece), index_to_look);
                result = TRUE;
                break;
            }
        }
    } else
    {
        if (piece->generate_legal_moves(piece, board, should_simulate))
        {
            for (unsigned long i = 0ul; i < piece->moves_number; ++i)
            {
                if (!SGLIB_QUEUE_IS_EMPTY(int, piece->possible_squares.index_array, piece->possible_squares.i, piece->possible_squares.j))
                {
                    int index_to_look = SGLIB_QUEUE_FIRST_ELEMENT(int, piece->possible_squares.index_array, piece->possible_squares.i, piece->possible_squares.j);
                    SGLIB_QUEUE_DELETE(int, piece->possible_squares.index_array, piece->possible_squares.i, piece->possible_squares.j, MAX_QUEUE_SIZE);
                    const cell_t *const cell_to_look = board->cells[index_to_look];

                    if (!SDL_memcmp(cell_to_look, destination, sizeof(cell_t)))
                    {
                        SDL_Log("[[CHECK CHECKMATE]]: [%s] [%s] can move on index: [%i]", player_color, chess_piece_to_string(piece), index_to_look);
                        result = TRUE;
                        break;
                    }
                }
            }
        }
    }

    return result;
}

chess_piece_t *chess_piece_new(piece_type_t type, char is_white, const char use_blending)
{
    chess_piece_t *piece = (chess_piece_t *)calloc(1, sizeof(chess_piece_t));
    CHECK(piece, NULL, "Couldn't allocate memory for chess_piece_t");

    piece->piece_type = type;
    piece->draw = _draw_piece;
    piece->set_position = _set_position;
    piece->generate_legal_moves = _generate_legal_moves;
    piece->check_checkmate = _check_checkmate;
    piece->piece_data.is_white = is_white;
    piece->piece_data.is_first_move = TRUE;
    piece->chess_texture = get_chess_texture(type, is_white, use_blending);

    // Setup score values for pieces
    switch (type)
    {
    case rook: piece->score_value = 5; break;
    case knight:
    case bishop: piece->score_value = 3; break;
    case queen: piece->score_value = 9; break;
    case king: piece->score_value = UINT_MAX; break;
    case pawn: piece->score_value = 1; break;
    default: break;
    }

    return piece;
}

void chess_piece_set_entity_cell(board_t *board, chess_piece_t *piece, int index)
{
    if (CHECK_IDX_RANGE(index))
    {
        SDL_Log("Index out of bounds: %i", index);
        return;
    }

    board->cells[index]->entity = piece;
    board->cells[index]->is_occupied = TRUE;
}

void chess_piece_set_entity_null(board_t *board, unsigned index)
{
    if (CHECK_IDX_RANGE(index))
    {
        SDL_Log("Index out of bounds: %i", index);
        return;
    }

    board->cells[index]->entity = NULL;
    board->cells[index]->is_occupied = FALSE;
}

char chess_piece_is_near_upper_bound(chess_piece_t *piece) { return piece != NULL && piece->pos_y == 0; }

char chess_piece_is_near_lower_bound(chess_piece_t *piece) { return piece != NULL && piece->pos_y == (SCREEN_H - CELL_SZ); }

char chess_piece_is_near_left_bound(chess_piece_t *piece) { return piece != NULL && piece->pos_x == 0; }

char chess_piece_is_near_right_bound(chess_piece_t *piece) { return piece != NULL && piece->pos_x == (SCREEN_W - CELL_SZ); }

void chess_piece_destroy(chess_piece_t *piece)
{
    texture_destroy(piece->chess_texture);

    for (size_t i = 0; i < piece->moves_number; i++)
    {
        piece_move_destroy(piece->moves[i]);
    }

    free(piece);
}

const char *chess_piece_to_string(chess_piece_t *piece)
{
    switch (piece->piece_type)
    {
    case rook: return "Rook";
    case knight: return "Knight";
    case bishop: return "Bishop";
    case queen: return "Queen";
    case king: return "King";
    case pawn: return "Pawn";
    default: return "**Invalid Pawn**";
    }
}