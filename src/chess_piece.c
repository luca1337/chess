#include "chess_piece.h"
#include "events.h"
#include "texture.h"
#include "board.h"
#include "player.h"
#include "cell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern events_t *events;
extern queue_t *texture_queue;

const char *white_png_postfix = "_w.png";
const char *black_png_postfix = "_b.png";

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

static void _draw_piece(struct chess_piece *piece)
{
    piece->chess_texture->render(piece->chess_texture, 0, 0, NULL);
}

static void _set_position(struct chess_piece *piece, int x, int y)
{
    piece->pos_x = x;
    piece->pos_y = y;
    piece->chess_texture->set_position(piece->chess_texture, x, y);
}

piece_move_t *piece_move_new()
{
    piece_move_t *move = (piece_move_t *)calloc(1, sizeof(piece_move_t));
    move->markers = queue_peek(texture_queue);
    queue_dequeue(texture_queue);
    return move;
}

void piece_move_destroy(piece_move_t* move)
{
    cell_destroy(move->possible_cells);
    texture_destroy(move->markers);
    free(move);
}

static int get_cell_index_by_piece_position(chess_piece_t* piece, int x_offset, int y_offset)
{
    return (((piece->pos_y / CELL_SZ) * CELLS_PER_ROW) + (piece->pos_x / CELL_SZ) + x_offset) + (y_offset * CELLS_PER_ROW);
}

static char allocate_legal_moves(chess_piece_t* piece, board_t* board, char simulate)
{
    // if no moves are available simply go out and this pawn cannot move.
    if (piece->moves_number == 0)
        return FALSE;

    // We distinguish the simulation because, in this case, we only care abouts "indexes" of the board
    // that the pawn could go if moved.
    if (simulate)
    {
        piece->possible_squares = (int*)calloc(piece->moves_number, sizeof(int) * piece->moves_number);
        CHECK(piece->possible_squares, NULL, "Couldn't allocate memory for piece->possible_squares");

        for (size_t i = 0; i < piece->moves_number; i++)
        {
            const int index = queue_peek(piece->index_queue);
            piece->possible_squares[i] = index;
            queue_dequeue(piece->index_queue);
        }
    }
    else
    {
        piece->moves = (piece_move_t **)calloc(piece->moves_number, sizeof(piece_move_t *));
        CHECK(piece->moves, NULL, "Couldn't allocate memory for piece->moves ");

        for (size_t i = 0; i < piece->moves_number; i++)
        {
            int index = queue_peek(piece->index_queue);

            piece->moves[i] = piece_move_new();
            piece->moves[i]->possible_cells = board->cells[index];
            piece->moves[i]->markers->set_position(piece->moves[i]->markers, piece->moves[i]->possible_cells->pos_x, piece->moves[i]->possible_cells->pos_y);

            queue_dequeue(piece->index_queue);
        }
    }

    return TRUE;
}

char get_pawn_legal_moves(chess_piece_t* piece, board_t* board, char simulate)
{
    // reset variables
    piece->moves_number = 0;

    const char is_first_move = piece->is_first_move;
    const char is_white = piece->is_white;

    // matrice in indici verticali
    const int vertical_moves_count = is_first_move ? 2 : 1;
    const int diagonal_moves_count = 2;

    // se è la prima volta che muovo la pedina, posso suggerire due caselle in verticale ( solo x muovermi e non x mangiare )
    // invece le caselle in diagonale possono sempre essere suggerite in quanto servono solo per mangiare e non per spostarsi liberamente
    piece->index_queue = queue_new(diagonal_moves_count + vertical_moves_count, sizeof(int) * (diagonal_moves_count + vertical_moves_count));

    // vertical squares
    const int first_vert_idx = piece->is_white ? get_cell_index_by_piece_position(piece, -0, -1) : get_cell_index_by_piece_position(piece, +0, +1);
    const int second_vert_idx = piece->is_white ? get_cell_index_by_piece_position(piece, -0, -2) : get_cell_index_by_piece_position(piece, +0, +2);

    // diagonal squares
    const int diagonal_left_idx = piece->is_white ? get_cell_index_by_piece_position(piece, -1, -1) : get_cell_index_by_piece_position(piece, -1, +1);
    const int diagonal_right_idx = piece->is_white ? get_cell_index_by_piece_position(piece, +1, -1) : get_cell_index_by_piece_position(piece, +1, +1);

    const int possible_squares[] = {
        diagonal_left_idx,
        diagonal_right_idx,
        first_vert_idx,
        second_vert_idx // questo viene controllato solo la prima volta.
    };

    // verticale e diagonale
    for (size_t squareIdx = 0; squareIdx != (diagonal_moves_count + vertical_moves_count); squareIdx++)
    {
        const int matrix_index = possible_squares[squareIdx];

        if (matrix_index < 0 || matrix_index > (BOARD_SZ - 1))
            continue;

        cell_t *current_cell = board->cells[matrix_index];

        if (squareIdx < diagonal_moves_count)
        {
            const char skip_lateral_bounds = squareIdx == 0 ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

            // controllo se la pedina è ai bordi della matrice per evitare l'overflow
            if (skip_lateral_bounds || !current_cell)
                continue;

            // arrivati qui siamo sicuri che la cella digonale NON sia nulla
            const chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

            // se la cella diagonale ha una pedina ed è del nostro stesso team o non esiste una pedina, non possiamo muoverci
            if ((current_cell_piece && (current_cell_piece->is_white == is_white)) || !current_cell_piece)
                continue;
        }
        else
        {
            // se la prima cella è invalicabile o nulla rompo il ciclo
            if (current_cell->is_occupied)
                break;
        }

        queue_enqueue(piece->index_queue, matrix_index);

        piece->moves_number++;
    }

    return allocate_legal_moves(piece, board, simulate);
}

char get_knight_legal_moves(chess_piece_t* piece, board_t* board, char simulate)
{
    // the knight can move in a "L" shape in all directions ->
    // controllo direttamente la casella all'indice calcolato per la matrice
    // se è fuori bounds rompo il ciclo

    // reset variables
    piece->moves_number = 0;

    const int max_moves = 10;

    piece->index_queue = queue_new(max_moves, sizeof(int) * max_moves);

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

    const char is_knight_within_lateral_bound_left = piece->pos_x >= 128;
    const char is_knight_within_lateral_bound_right = piece->pos_x <= 320;

    move_direction_t move_direction = east;
    while (move_direction != MAX_DIR)
    {
        for(ever)
        {
            int possible_square = 0;

            switch (move_direction)
            {
            default:                                                    break;
            case east:          possible_square     = left_up_idx;      break;
            case north_east:    possible_square     = up_left_idx;      break;
            case north:         possible_square     = up_right_idx;     break;
            case north_west:    possible_square     = right_up_idx;     break;
            case west:          possible_square     = right_down_idx;   break;
            case south_west:    possible_square     = down_right_idx;   break;
            case south:         possible_square     = down_left_idx;    break;
            case south_east:    possible_square     = left_down_idx;    break;
            }

            if (possible_square < 0 || possible_square > (BOARD_SZ - 1))
                break;

            const char is_east            = move_direction == east;
            const char is_north_east      = move_direction == north_east;
            const char is_south           = move_direction == south;
            const char is_south_east      = move_direction == south_east;
            const char is_north           = move_direction == north;
            const char is_north_west      = move_direction == north_west;
            const char is_west            = move_direction == west;
            const char is_south_west      = move_direction == south_west;

            if (is_south && !(piece->pos_x >= 64))
                break;
            
            if (is_north && !(piece->pos_x <= 384))
                break;

            if ((is_east || is_north_east || is_south_east) && !is_knight_within_lateral_bound_left)
                    break;

            if ((is_north_west || is_west || is_south_west) && !is_knight_within_lateral_bound_right)
                    break;

            cell_t *current_cell = board->cells[possible_square];
            chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

            if (!current_cell->is_occupied || (current_cell->is_occupied && ((current_cell_piece->is_white != piece->is_white))))
            {
                queue_enqueue(piece->index_queue, possible_square);
                piece->moves_number++;
                break;
            }

            if (current_cell->is_occupied && (current_cell_piece->is_white == piece->is_white))
                break;
        }

        move_direction++;
    }

    return allocate_legal_moves(piece, board, simulate);
}

char get_queen_legal_moves(chess_piece_t* piece, board_t* board, char simulate)
{
    piece->moves_number = 0;

    const int max_steps = 7;
    const int max_queen_moves = max_steps * MAX_DIR;

    piece->index_queue = queue_new(max_queen_moves, sizeof(int) * max_queen_moves);

    // indice corrente della pedina nella matrice 8x8
    int piece_index = ((piece->pos_y / CELL_SZ) * CELLS_PER_ROW) + (piece->pos_x / CELL_SZ);

    // the queen can move along all the six direction as far as possible until she encounter her ally or an enemy
    // we must check every possible direction starting from the nearest one

    // turning from east to sout-east all aorund in a clock-wise order
    move_direction_t move_direction = east;

    int step = 1;
    while (move_direction != MAX_DIR)
    {
        int matrix_index = 0;
        for(ever)
        {
            if (move_direction == east || move_direction == west)
            {
                char is_east = move_direction == east;
                char is_near_lateral_bounds = is_east ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

                if (is_near_lateral_bounds)
                    break;

                matrix_index = is_east ? (piece_index - step) : (piece_index + step);

                cell_t *current_cell = board->cells[matrix_index];
                chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

                char is_current_cell_near_bounds = is_east ? is_cell_left_bound(current_cell) : is_cell_right_bound(current_cell);

                if (is_current_cell_near_bounds && ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white) || !current_cell->is_occupied))
                {
                    queue_enqueue(piece->index_queue, matrix_index);
                    piece->moves_number++;
                    break;
                }

                if (current_cell->is_occupied && current_cell_piece->is_white != piece->is_white)
                {
                    queue_enqueue(piece->index_queue, matrix_index);
                    piece->moves_number++;
                    break;
                }

                if ((current_cell->is_occupied && current_cell_piece->is_white == piece->is_white))
                    break;

                queue_enqueue(piece->index_queue, matrix_index);

                step++;
                piece->moves_number++;
            }
            else if (move_direction == north_east || move_direction == north_west)
            {
                char is_north_east = move_direction == north_east;
                char is_near_lateral_bounds = is_north_east ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

                matrix_index = is_north_east ? (piece_index - 1) - CELLS_PER_ROW : (piece_index + 1) - CELLS_PER_ROW;

                if (is_near_lateral_bounds || matrix_index < 0)
                    break;

                cell_t *current_cell = board->cells[matrix_index];
                chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

                char is_cell_near_lateral_bounds = is_north_east ? is_cell_left_bound(current_cell) : is_cell_right_bound(current_cell);

                if (is_cell_near_lateral_bounds && ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white) || !current_cell->is_occupied))
                {
                    queue_enqueue(piece->index_queue, matrix_index);
                    piece->moves_number++;
                    break;
                }

                if (current_cell->is_occupied && current_cell_piece->is_white != piece->is_white)
                {
                    queue_enqueue(piece->index_queue, matrix_index);
                    piece->moves_number++;
                    break;
                }

                if ((current_cell->is_occupied && current_cell_piece->is_white == piece->is_white))
                    break;

                queue_enqueue(piece->index_queue, matrix_index);

                piece_index = matrix_index;

                step++;
                piece->moves_number++;
            }
            else if (move_direction == north || move_direction == south)
            {
                char is_north = move_direction == north;

                matrix_index = is_north ? (piece_index - (CELLS_PER_ROW * step)) : (piece_index + (CELLS_PER_ROW * step));

                if (matrix_index < 0 || matrix_index > (BOARD_SZ - 1))
                    break;

                cell_t *current_cell = board->cells[matrix_index];
                chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

                if ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white))
                {
                    piece->moves_number++;
                    queue_enqueue(piece->index_queue, matrix_index);
                    break;
                }

                if ((current_cell->is_occupied && current_cell_piece->is_white == piece->is_white))
                    break;

                queue_enqueue(piece->index_queue, matrix_index);

                step++;
                piece->moves_number++;
            }
            else if (move_direction == south_west || move_direction == south_east)
            {
                char is_south_west = move_direction == south_west;
                char is_queen_near_lateral_bounds = is_south_west ? chess_piece_is_near_right_bound(piece) : chess_piece_is_near_left_bound(piece);

                matrix_index = is_south_west ? (piece_index + 1) + CELLS_PER_ROW : (piece_index - 1) + CELLS_PER_ROW;

                if (is_queen_near_lateral_bounds || matrix_index > (BOARD_SZ - 1))
                    break;

                cell_t *current_cell = board->cells[matrix_index];
                chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

                char is_cell_near_lateral_bounds = is_south_west ? is_cell_right_bound(current_cell) : is_cell_left_bound(current_cell);

                if (is_cell_near_lateral_bounds && ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white) || !current_cell->is_occupied))
                {
                    queue_enqueue(piece->index_queue, matrix_index);
                    piece->moves_number++;
                    break;
                }

                if ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white))
                {
                    queue_enqueue(piece->index_queue, matrix_index);
                    piece->moves_number++;
                    break;
                }

                if ((current_cell->is_occupied && current_cell_piece->is_white == piece->is_white))
                    break;

                queue_enqueue(piece->index_queue, matrix_index);

                piece_index = matrix_index;

                piece->moves_number++;
                step++;
            }
            else break;
        }

        piece_index = ((piece->pos_y / CELL_SZ) * CELLS_PER_ROW) + (piece->pos_x / CELL_SZ);
        step = 1;
        move_direction++;
    }

    return allocate_legal_moves(piece, board, simulate);
}

char get_rook_legal_moves(chess_piece_t* piece, board_t* board, char simulate)
{
    piece->moves_number = 0;

    const int max_steps = 7;
    const int max_directions = 4;
    const int max_rook_moves = max_steps * max_directions;

    piece->index_queue = queue_new(max_rook_moves, sizeof(int) * max_rook_moves);

    // indice corrente della pedina nella matrice 8x8
    int piece_index = ((piece->pos_y / CELL_SZ) * CELLS_PER_ROW) + (piece->pos_x / CELL_SZ);

    // the queen can move along all the six direction as far as possible until she encounter her ally or an enemy
    // we must check every possible direction starting from the nearest one

    move_direction_t move_directions[4] = { east, north, west, south };

    int step = 1;
    for (unsigned long dirIdx = 0ul; dirIdx != _countof(move_directions); ++dirIdx)
    {
        move_direction_t current_dir = move_directions[dirIdx];
        int cell_index = 0;
        for(ever)
        {
            if (current_dir == east || current_dir == west)
            {
                char is_east = current_dir == east;
                char is_near_lateral_bounds = is_east ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

                if (is_near_lateral_bounds)
                    break;

                cell_index = is_east ? (piece_index - step) : (piece_index + step);

                cell_t *current_cell = board->cells[cell_index];
                chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

                char is_current_cell_near_bounds = is_east ? is_cell_left_bound(current_cell) : is_cell_right_bound(current_cell);

                if (is_current_cell_near_bounds && ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white) || !current_cell->is_occupied))
                {
                    queue_enqueue(piece->index_queue, cell_index);
                    piece->moves_number++;
                    break;
                }

                if (current_cell->is_occupied && current_cell_piece->is_white != piece->is_white)
                {
                    queue_enqueue(piece->index_queue, cell_index);
                    piece->moves_number++;
                    break;
                }

                if ((current_cell->is_occupied && current_cell_piece->is_white == piece->is_white))
                    break;

                queue_enqueue(piece->index_queue, cell_index);

                step++;
                piece->moves_number++;
            }
            else
            {
                char is_north = current_dir == north;

                cell_index = is_north ? (piece_index - (CELLS_PER_ROW * step)) : (piece_index + (CELLS_PER_ROW * step));

                if (cell_index < 0 || cell_index > (BOARD_SZ - 1))
                    break;

                cell_t *current_cell = board->cells[cell_index];
                chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

                if ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white))
                {
                    piece->moves_number++;
                    queue_enqueue(piece->index_queue, cell_index);
                    break;
                }

                if ((current_cell->is_occupied && current_cell_piece->is_white == piece->is_white))
                    break;

                queue_enqueue(piece->index_queue, cell_index);

                step++;
                piece->moves_number++;
            }
        }

        piece_index = ((piece->pos_y / CELL_SZ) * CELLS_PER_ROW) + (piece->pos_x / CELL_SZ);
        step = 1;
    }
    

    return allocate_legal_moves(piece, board, simulate);
}

char get_bishop_legal_moves(chess_piece_t* piece, board_t* board, char simulate)
{
    piece->moves_number = 0;

    const int max_steps = 7;
    const int max_directions = 4;
    const int max_bishop_moves = max_steps * max_directions;

    piece->index_queue = queue_new(max_bishop_moves, sizeof(int) * max_bishop_moves);

    // indice corrente della pedina nella matrice 8x8
    int piece_index = ((piece->pos_y / CELL_SZ) * CELLS_PER_ROW) + (piece->pos_x / CELL_SZ);

    const move_direction_t move_directions[4] = { north_east, north_west, south_east, south_west };

    int step = 1;
    for (unsigned long dirIdx = 0ul; dirIdx != _countof(move_directions); ++dirIdx)
    {
        move_direction_t current_dir = move_directions[dirIdx];
        int cell_index = 0;
        for(ever)
        {
            if (current_dir == north_east || current_dir == north_west)
            {
                char is_north_east = current_dir == north_east;
                char is_near_lateral_bounds = is_north_east ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

                cell_index = is_north_east ? (piece_index - 1) - CELLS_PER_ROW : (piece_index + 1) - CELLS_PER_ROW;

                if (is_near_lateral_bounds || cell_index < 0)
                    break;

                cell_t *current_cell = board->cells[cell_index];
                chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

                char is_cell_near_lateral_bounds = is_north_east ? is_cell_left_bound(current_cell) : is_cell_right_bound(current_cell);

                if (is_cell_near_lateral_bounds && ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white) || !current_cell->is_occupied))
                {
                    queue_enqueue(piece->index_queue, cell_index);
                    piece->moves_number++;
                    break;
                }

                if (current_cell->is_occupied && current_cell_piece->is_white != piece->is_white)
                {
                    queue_enqueue(piece->index_queue, cell_index);
                    piece->moves_number++;
                    break;
                }

                if ((current_cell->is_occupied && current_cell_piece->is_white == piece->is_white))
                    break;

                queue_enqueue(piece->index_queue, cell_index);

                piece_index = cell_index;

                step++;
                piece->moves_number++;
            }
            else
            {
                char is_south_west = current_dir == south_west;
                char is_bishop_near_lateral_bounds = is_south_west ? chess_piece_is_near_right_bound(piece) : chess_piece_is_near_left_bound(piece);

                cell_index = is_south_west ? (piece_index + 1) + CELLS_PER_ROW : (piece_index - 1) + CELLS_PER_ROW;

                if (is_bishop_near_lateral_bounds || cell_index > (BOARD_SZ - 1))
                    break;

                cell_t *current_cell = board->cells[cell_index];
                chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

                char is_cell_near_lateral_bounds = is_south_west ? is_cell_right_bound(current_cell) : is_cell_left_bound(current_cell);

                if (is_cell_near_lateral_bounds && ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white) || !current_cell->is_occupied))
                {
                    queue_enqueue(piece->index_queue, cell_index);
                    piece->moves_number++;
                    break;
                }

                if ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white))
                {
                    queue_enqueue(piece->index_queue, cell_index);
                    piece->moves_number++;
                    break;
                }

                if ((current_cell->is_occupied && current_cell_piece->is_white == piece->is_white))
                    break;

                queue_enqueue(piece->index_queue, cell_index);

                piece_index = cell_index;

                piece->moves_number++;
                step++;
            }
        }

        piece_index = ((piece->pos_y / CELL_SZ) * CELLS_PER_ROW) + (piece->pos_x / CELL_SZ);
        step = 1;
    }
    
    return allocate_legal_moves(piece, board, simulate);
}

char get_king_legal_moves(chess_piece_t* piece, board_t* board, char simulate)
{
    // the king can move to adjacent cells in all directions, by one square.
    piece->moves_number = 0;

    const int max_moves = 8;

    piece->index_queue = queue_new(max_moves, sizeof(int) * max_moves);

    // precalculate possible squares
    const int lefx_idx        = get_cell_index_by_piece_position(piece, -1, -0);
    const int up_left_idx     = get_cell_index_by_piece_position(piece, -1, -1);
    const int up_idx          = get_cell_index_by_piece_position(piece, -0, -1);
    const int up_right_idx    = get_cell_index_by_piece_position(piece, +1, -1);
    const int right_idx       = get_cell_index_by_piece_position(piece, +1, +0);
    const int down_right_idx  = get_cell_index_by_piece_position(piece, +1, +1);
    const int down_idx        = get_cell_index_by_piece_position(piece, +0, +1);
    const int down_left_idx   = get_cell_index_by_piece_position(piece, -1, +1);

    move_direction_t move_direction = east;
    while (move_direction != MAX_DIR)
    {
        for(ever)
        {
            int possible_square = 0;

            switch (move_direction)
            {
            default:                                                    break;
            case east:          possible_square     = lefx_idx;         break;
            case north_east:    possible_square     = up_left_idx;      break;
            case north:         possible_square     = up_idx;           break;
            case north_west:    possible_square     = up_right_idx;     break;
            case west:          possible_square     = right_idx;        break;
            case south_west:    possible_square     = down_right_idx;   break;
            case south:         possible_square     = down_idx;         break;
            case south_east:    possible_square     = down_left_idx;    break;
            }

            const char is_east            = move_direction == east;
            const char is_north_east      = move_direction == north_east;
            const char is_south_east      = move_direction == south_east;
            const char is_north_west      = move_direction == north_west;
            const char is_west            = move_direction == west;
            const char is_south_west      = move_direction == south_west;

            if ((is_east || is_north_east || is_south_east) && chess_piece_is_near_left_bound(piece))
                    break;

            if ((is_north_west || is_west || is_south_west) && chess_piece_is_near_right_bound(piece))
                    break;

            if (possible_square < 0 || possible_square > (BOARD_SZ - 1))
                break;

            cell_t *const current_cell = board->cells[possible_square];

            // check checkmate: to implement a check checkmate algorithm, we need to check for each piece present on the board
            // if it can occupy one of the hypotetical available cells for the king

            const chess_piece_t *const current_cell_piece = (chess_piece_t *)current_cell->entity;

            if (!current_cell->is_occupied || (current_cell->is_occupied && ((current_cell_piece->is_white != piece->is_white))))
            {
                // this statement because when the piece currently checked it's a king then this function will be called recursively
                if (!simulate)
                {
                    // only here the king could move but we must check if that cell will turn him into checkmate
                    // we must iterate over all the enemies' pawns and check whether they could reach this cell
                    for (unsigned long cellIdx = 0ul; cellIdx != BOARD_SZ; ++cellIdx)
                    {
                        chess_piece_t *enemy_pc = (chess_piece_t *)board->cells[cellIdx]->entity;

                        if (!enemy_pc)
                            continue;

                        if (piece->is_white == enemy_pc->is_white)
                            continue;

                        // we stop everytime a enemy piece can move in the same cell that the king could move to
                        if (enemy_pc->check_checkmate(board, enemy_pc, current_cell))
                            goto exit_loop; // jmp
                    }
                }
                
                queue_enqueue(piece->index_queue, possible_square);
                piece->moves_number++;
                break;
            }

            if (current_cell->is_occupied && (current_cell_piece->is_white == piece->is_white))
                break;
        }
        exit_loop:

        move_direction++;
    }

    return allocate_legal_moves(piece, board, simulate);
}

char _generate_legal_moves(struct chess_piece *piece, board_t *board)
{
    // algoritmi di ricerca celle disponibili

    const char should_simulate = FALSE;
    switch (piece->piece_type)
    {
    default:        break;
    case rook:      return get_rook_legal_moves(piece, board, should_simulate);
    case knight:    return get_knight_legal_moves(piece, board, should_simulate);
    case bishop:    return get_bishop_legal_moves(piece, board, should_simulate);
    case queen:     return get_queen_legal_moves(piece, board, should_simulate);
    case king:      return get_king_legal_moves(piece, board, should_simulate);
    case pawn:      return get_pawn_legal_moves(piece, board, should_simulate);
    }

    return FALSE;
}

char _check_checkmate(board_t* board, struct chess_piece* piece, cell_t* destination)
{
    // the checkmate algorithm it's nothing more than a simulation of the legal move's one
    // for every enemy piece that could potentially reach any of the king's cell, so we call
    // get_piece_legal_moves it's done.

    const char should_simulate = TRUE;
    const piece_type_t piece_type = piece->piece_type;
    char should_check = FALSE;

    switch (piece_type)
    {
        default: break;
        case rook:      should_check = get_rook_legal_moves(piece, board, should_simulate); break;
        case knight:    should_check = get_knight_legal_moves(piece, board, should_simulate); break;
        case bishop:    should_check = get_bishop_legal_moves(piece, board, should_simulate); break;
        case queen:     should_check = get_queen_legal_moves(piece, board, should_simulate); break;
        case king:      should_check = get_king_legal_moves(piece, board, should_simulate); break;
        case pawn:
        {
            const int possible_moves_count = 2;

            const int diagonal_left_idx = piece->is_white ? get_cell_index_by_piece_position(piece, -1, -1) : get_cell_index_by_piece_position(piece, -1, +1);
            const int diagonal_right_idx = piece->is_white ? get_cell_index_by_piece_position(piece, +1, -1) : get_cell_index_by_piece_position(piece, +1, +1);
            
            const int possible_moves[] = {
                diagonal_left_idx,
                diagonal_right_idx
            };

            for (unsigned long moveIdx = 0ul; moveIdx != possible_moves_count; ++moveIdx)
            {
                int index_to_look = possible_moves[moveIdx];
                char skip_lateral_bounds = moveIdx == 0 ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

                if (index_to_look < 0 || index_to_look > (BOARD_SZ - 1) || skip_lateral_bounds)
                    continue;

                const cell_t *const cell_to_look = board->cells[index_to_look];

                if (!memcmp(cell_to_look, destination, sizeof(cell_t)))
                {
                    fprintf(stdout, "[[WARNING]]: [%s] could move at index: [%i]\n", chess_piece_to_string(piece), index_to_look);
                    return TRUE;
                }
            }
        } 
        break;
    }
        
    if (should_check)
    {
        for (size_t i = 0; i < piece->moves_number; i++)
        {
            const int index_to_look = piece->possible_squares[i];
            const cell_t *const cell_to_look = board->cells[index_to_look];

            if (!memcmp(cell_to_look, destination, sizeof(cell_t)) && !cell_to_look->is_occupied)
            {
                fprintf(stdout, "[[WARNING]]: [%s] could move at index: [%i]\n", chess_piece_to_string(piece), index_to_look);
                return TRUE;
            }
        }

        // free(piece->possible_squares);
    }

    return FALSE;
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
    piece->is_white = is_white;
    piece->is_first_move = TRUE;
    piece->chess_texture = get_chess_texture(type, is_white, use_blending);

    switch (type)
    {
    case rook: piece->score_value = 5; break;
    case knight: case bishop: piece->score_value = 3; break;
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
        printf("Index out of bounds: %i\n", index);
        return;
    }

    board->cells[index]->entity = piece;
    board->cells[index]->is_occupied = TRUE;
}

void chess_piece_set_entity_null(board_t *board, unsigned index)
{
    if (CHECK_IDX_RANGE(index))
    {
        printf("Index out of bounds: %i\n", index);
        return;
    }

    board->cells[index]->entity = NULL;
    board->cells[index]->is_occupied = FALSE;
}

char chess_piece_is_near_upper_bound(chess_piece_t *piece)
{
    return piece != NULL && piece->pos_y == 0;
}

char chess_piece_is_near_lower_bound(chess_piece_t *piece)
{
    return piece != NULL && piece->pos_y == (SCREEN_H - CELL_SZ);
}

char chess_piece_is_near_left_bound(chess_piece_t *piece)
{
    return piece != NULL && piece->pos_x == 0;
}

char chess_piece_is_near_right_bound(chess_piece_t *piece)
{
    return piece != NULL && piece->pos_x == (SCREEN_W - CELL_SZ);
}

void chess_piece_destroy(chess_piece_t* piece)
{
    texture_destroy(piece->chess_texture);

    for (size_t i = 0; i < piece->moves_number; i++)
    {
        piece_move_destroy(piece->moves[i]);
    }
    
    free(piece);
}

const char* chess_piece_to_string(chess_piece_t* piece)
{
    switch (piece->piece_type)
    {
    case rook:      return "Rook";
    case knight:    return "Knight";
    case bishop:    return "Bishop";
    case queen:     return "Queen";
    case king:      return "King";
    case pawn:      return "Pawn";
    default:        return "**Invalid Pawn**";
    }
}