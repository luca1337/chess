#include "chess_piece.h"
#include "events.h"
#include "queue.h"
#include "texture.h"

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

static texture_t *get_chess_texture(piece_type_t type, char is_white)
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

    result = texture_load_from_file(path);

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

static int get_matrix_index_from_piece_position(chess_piece_t* piece, int x_offset, int y_offset)
{
    return (((piece->pos_y / CELL_SZ) * CELLS_PER_ROW) + (piece->pos_x / CELL_SZ) + x_offset) + (y_offset * CELLS_PER_ROW);
}

piece_move_t **_get_moves(struct chess_piece *piece, board_t *board)
{
    // variabili comuni
    size_t available_moves = 0;
    piece_move_t **moves = NULL;
    queue_t *index_queue = NULL;

    const char is_pawn      = piece->piece_type == pawn;
    const char is_rook      = piece->piece_type == rook;
    const char is_bishop    = piece->piece_type == bishop;
    const char is_queen     = piece->piece_type == queen;
    const char is_knight    = piece->piece_type == knight;
    const char is_king      = piece->piece_type == king;

    // algoritmi di ricerca celle disponibili
    if (is_pawn)
    {
        char is_first_move = piece->is_first_move;
        char is_white = piece->is_white;

        // matrice in indici verticali
        int vertical_moves_count = is_first_move ? 2 : 1;
        const int diagonal_moves_count = 2;

        // se è la prima volta che muovo la pedina, posso suggerire due caselle in verticale ( solo x muovermi e non x mangiare )
        // invece le caselle in diagonale possono sempre essere suggerite in quanto servono solo per mangiare e non per spostarsi liberamente
        index_queue = queue_new(diagonal_moves_count + vertical_moves_count, sizeof(int) * (diagonal_moves_count + vertical_moves_count));

        // vertical squares
        int first_vert_idx = piece->is_white ? get_matrix_index_from_piece_position(piece, -0, -1) : get_matrix_index_from_piece_position(piece, +0, +1);
        int second_vert_idx = piece->is_white ? get_matrix_index_from_piece_position(piece, -0, -2) : get_matrix_index_from_piece_position(piece, +0, +2);

        // diagonal squares
        int diagonal_left_idx = piece->is_white ? get_matrix_index_from_piece_position(piece, -1, -1) : get_matrix_index_from_piece_position(piece, -1, +1);
        int diagonal_right_idx = piece->is_white ? get_matrix_index_from_piece_position(piece, +1, -1) : get_matrix_index_from_piece_position(piece, +1, +1);

        int possible_squares[] = {
            diagonal_left_idx,
            diagonal_right_idx,
            first_vert_idx,
            second_vert_idx // questo viene controllato solo la prima volta.
        };

        // verticale e diagonale
        for (size_t squareIdx = 0; squareIdx != (diagonal_moves_count + vertical_moves_count); squareIdx++)
        {
            int matrix_index = possible_squares[squareIdx];

            if (matrix_index < 0 || matrix_index > (BOARD_SZ - 1))
                continue;

            cell_t *current_cell = board->cells[matrix_index];

            if (squareIdx < diagonal_moves_count)
            {
                char skip_lateral_bounds = squareIdx == 0 ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

                // controllo se la pedina è ai bordi della matrice per evitare l'overflow
                if (skip_lateral_bounds || !current_cell)
                    continue;

                // arrivati qui siamo sicuri che la cella digonale NON sia nulla
                chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

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

            queue_enqueue(index_queue, matrix_index);

            available_moves++;
        }
    }
    else if (is_queen || is_rook || is_bishop)
    {
        const int max_steps = 7;
        const int max_queen_moves = max_steps * MAX_DIR;

        index_queue = queue_new(max_queen_moves, sizeof(int) * max_queen_moves);

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
                if ((is_queen || is_rook) && (move_direction == east || move_direction == west))
                {
                    char is_east = move_direction == east;
                    char is_queen_near_bounds = is_east ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

                    if (is_queen_near_bounds)
                        break;

                    matrix_index = is_east ? (piece_index - step) : (piece_index + step);

                    cell_t *current_cell = board->cells[matrix_index];
                    chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

                    char is_current_cell_near_bounds = is_east ? is_cell_left_bound(current_cell) : is_cell_right_bound(current_cell);

                    if (is_current_cell_near_bounds && ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white) || !current_cell->is_occupied))
                    {
                        queue_enqueue(index_queue, matrix_index);
                        available_moves++;
                        break;
                    }

                    if (current_cell->is_occupied && current_cell_piece->is_white != piece->is_white)
                    {
                        queue_enqueue(index_queue, matrix_index);
                        available_moves++;
                        break;
                    }

                    if ((current_cell->is_occupied && current_cell_piece->is_white == piece->is_white))
                        break;

                    queue_enqueue(index_queue, matrix_index);

                    step++;
                    available_moves++;
                }
                else if ((is_queen || is_bishop) && (move_direction == north_east || move_direction == north_west))
                {
                    char is_north_east = move_direction == north_east;
                    char is_queen_near_lateral_bounds = is_north_east ? chess_piece_is_near_left_bound(piece) : chess_piece_is_near_right_bound(piece);

                    matrix_index = is_north_east ? (piece_index - 1) - CELLS_PER_ROW : (piece_index + 1) - CELLS_PER_ROW;

                    if (is_queen_near_lateral_bounds || matrix_index < 0)
                        break;

                    cell_t *current_cell = board->cells[matrix_index];
                    chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

                    char is_cell_near_lateral_bounds = is_north_east ? is_cell_left_bound(current_cell) : is_cell_right_bound(current_cell);

                    if (is_cell_near_lateral_bounds && ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white) || !current_cell->is_occupied))
                    {
                        queue_enqueue(index_queue, matrix_index);
                        available_moves++;
                        break;
                    }

                    if (current_cell->is_occupied && current_cell_piece->is_white != piece->is_white)
                    {
                        queue_enqueue(index_queue, matrix_index);
                        available_moves++;
                        break;
                    }

                    if ((current_cell->is_occupied && current_cell_piece->is_white == piece->is_white))
                        break;

                    queue_enqueue(index_queue, matrix_index);

                    piece_index = matrix_index;

                    step++;
                    available_moves++;
                }
                else if ((is_queen || is_rook) && (move_direction == north || move_direction == south))
                {
                    char is_north = move_direction == north;

                    matrix_index = is_north ? (piece_index - (CELLS_PER_ROW * step)) : (piece_index + (CELLS_PER_ROW * step));

                    if (matrix_index < 0 || matrix_index > (BOARD_SZ - 1))
                        break;

                    cell_t *current_cell = board->cells[matrix_index];
                    chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

                    if ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white))
                    {
                        available_moves++;
                        queue_enqueue(index_queue, matrix_index);
                        break;
                    }

                    if ((current_cell->is_occupied && current_cell_piece->is_white == piece->is_white))
                        break;

                    queue_enqueue(index_queue, matrix_index);

                    step++;
                    available_moves++;
                }
                else if ((is_queen || is_bishop) && (move_direction == south_west || move_direction == south_east))
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
                        available_moves++;
                        queue_enqueue(index_queue, matrix_index);
                        break;
                    }

                    if ((current_cell->is_occupied && current_cell_piece->is_white != piece->is_white))
                    {
                        available_moves++;
                        queue_enqueue(index_queue, matrix_index);
                        break;
                    }

                    if ((current_cell->is_occupied && current_cell_piece->is_white == piece->is_white))
                        break;

                    queue_enqueue(index_queue, matrix_index);

                    piece_index = matrix_index;

                    step++;
                    available_moves++;
                }
                else
                    break;
            }

            // reset stuff after a direction is checked
            piece_index = ((piece->pos_y / CELL_SZ) * CELLS_PER_ROW) + (piece->pos_x / CELL_SZ);
            step = 1;
            move_direction++;
        }
    }
    else if (is_knight)
    {
        // the knight can move in a "L" shape in all directions ->
        // controllo direttamente la casella all'indice calcolato per la matrice
        // se è fuori bounds rompo il ciclo

        const int max_moves = 10;

        index_queue = queue_new(max_moves, sizeof(int) * max_moves);

        // up left, left up
        int left_up_idx = get_matrix_index_from_piece_position(piece, -2, -1);
        int up_left_idx = get_matrix_index_from_piece_position(piece, -1, -2);

        // up right, right up
        int up_right_idx = get_matrix_index_from_piece_position(piece, +1, -2);
        int right_up_idx = get_matrix_index_from_piece_position(piece, +2, -1);

        // right down, down right
        int right_down_idx = get_matrix_index_from_piece_position(piece, +2, +1);
        int down_right_idx = get_matrix_index_from_piece_position(piece, +1, +2);

        // down left, left down
        int down_left_idx = get_matrix_index_from_piece_position(piece, -1, +2);
        int left_down_idx = get_matrix_index_from_piece_position(piece, -2, +1);

        char is_knight_within_lateral_bound_left = piece->pos_x >= 128;
        char is_knight_within_lateral_bound_right = piece->pos_x <= 320;

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

                char is_east            = move_direction == east;
                char is_north_east      = move_direction == north_east;
                char is_south           = move_direction == south;
                char is_south_east      = move_direction == south_east;
                char is_north           = move_direction == north;
                char is_north_west      = move_direction == north_west;
                char is_west            = move_direction == west;
                char is_south_west      = move_direction == south_west;

                if ((is_east || is_north_east || is_south_east || is_south) && !is_knight_within_lateral_bound_left)
                     break;

                if ((is_north || is_north_west || is_west || is_south_west) && !is_knight_within_lateral_bound_right)
                     break;

                cell_t *current_cell = board->cells[possible_square];
                chess_piece_t *current_cell_piece = (chess_piece_t *)current_cell->entity;

                if (!current_cell->is_occupied || (current_cell->is_occupied && ((current_cell_piece->is_white != piece->is_white))))
                {
                    queue_enqueue(index_queue, possible_square);
                    available_moves++;
                    break;
                }

                if (current_cell->is_occupied && (current_cell_piece->is_white == piece->is_white))
                    break;
            }

            move_direction++;
        }
    }

    // esco se non ho mosse disponibili, sono bloccato
    if (available_moves == 0)
        return NULL;

    // inizializzo un puntatore ad un puntatore di mosse da suggerire della grandezza che abbiamo calcolato
    moves = (piece_move_t **)calloc(available_moves, sizeof(piece_move_t *));
    if (!moves)
    {
        fprintf(stderr, "Couldn't allocate memory for piece_move_t struct\n");
        return NULL;
    }

    for (size_t i = 0; i < available_moves; i++)
    {
        int index = queue_peek(index_queue);

        moves[i] = piece_move_new();
        moves[i]->possible_cells = board->cells[index];
        moves[i]->markers->set_position(moves[i]->markers, moves[i]->possible_cells->pos_x, moves[i]->possible_cells->pos_y);

        queue_dequeue(index_queue);
    }

    // libero la memoria che non utilizzo più per evitare mem leak!
    free(index_queue);
    index_queue = NULL;

    piece->moves_number = available_moves;

    return moves;
}

chess_piece_t *chess_piece_new(piece_type_t type, char is_white)
{
    chess_piece_t *piece = (chess_piece_t *)malloc(sizeof(chess_piece_t));
    memset(piece, 0, sizeof(chess_piece_t));

    piece->piece_type = type;
    piece->chess_texture = get_chess_texture(type, is_white);
    piece->draw = _draw_piece;
    piece->set_position = _set_position;
    piece->is_white = is_white;
    piece->is_first_move = TRUE;
    piece->get_moves = _get_moves;

    return piece;
}

void chess_piece_set_entity_cell(board_t *board, chess_piece_t *piece, int index)
{
    if (CHECK_IDX_RANGE(index))
    {
        printf("Index out of bounds: %i\n", index);
        return;
    }

    board->cells[index]->entity = (entity_t *)piece;
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