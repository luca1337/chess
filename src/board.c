#include "board.h"
#include "events.h"
#include "chess_piece.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <Windows.h>

extern renderer_t *renderer;
extern window_t *window;
extern events_t *events;

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
    entity_destroy(cell->entity);
}

static void _draw_board(struct board *board)
{
    // draw board
    for (unsigned long cellIndex = 0ul; cellIndex != BOARD_SZ; ++cellIndex)
    {
        cell_t *current_cell = board->cells[cellIndex];

        if (current_cell != NULL)
        {
            current_cell->draw(current_cell);
        }
    }

    // draw pieces
    for (unsigned long chessIndex = 0ul; chessIndex < BOARD_SZ; chessIndex++)
    {
        chess_piece_t *current_piece = (chess_piece_t *)board->cells[chessIndex]->entity;

        if (current_piece != NULL)
        {
            current_piece->draw(current_piece);
        }
    }
}

static void create_chess_piece(board_t *board, unsigned index, vec2_t position, piece_type_t type, char is_upper_board)
{
    chess_piece_t *piece = chess_piece_new(type, !is_upper_board);
    piece->set_position(piece, position.xy[0], position.xy[1]);
    board->cells[index]->entity = (entity_t *)piece;
    board->cells[index]->is_occupied = 1;
}

board_t *board_new()
{
    board_t *board = (board_t *)calloc(1, sizeof(board_t));
    CHECK(board, NULL, "Could not allocate memory for board");

    board->draw = _draw_board;

    color_t cell_color = color_create(0.0f, 0.0f, 0.0f, 1.0f);

    // piazziamo le celle della scacchiera
    for (size_t columnIndex = 0; columnIndex != CELLS_PER_ROW; ++columnIndex)
    {
        char is_odd = (columnIndex % 2);
        char is_black = is_odd;

        for (size_t rowIndex = 0; rowIndex != CELLS_PER_ROW; ++rowIndex)
        {
            // transform b-dim array to mono dimensional
            int cell_index = (columnIndex * CELLS_PER_ROW) + rowIndex;

            int pos_x = (cell_index % CELLS_PER_ROW) * CELL_SZ;
            int pos_y = (cell_index / CELLS_PER_ROW) * CELL_SZ;

            vec2_t position = vec2_create(pos_x, pos_y);
            vec2_t cell_size = vec2_create(CELL_SZ, CELL_SZ);

            // swap color based on cell oddity/evenly
            cell_color = is_black ? BLACK : WHITE;
            is_black = !is_black;

            // Dopo che abbiamo calcolato correttamente l'indice in cui la cella deve essere posizionata per formare
            // la scacchiera, e dopo aver calcolato anche il colore in base all'indice pari o dispari, creiamo le celle
            // passandogli posizione, grandezza e colore
            board->cells[cell_index] = cell_new(position, cell_size, cell_color);
            board->cells[cell_index]->pos_x = pos_x;
            board->cells[cell_index]->pos_y = pos_y;

            int board_matrix_value  = board_matrix[cell_index];
            piece_type_t type       = (piece_type_t)board_matrix_value;
            char is_upper_board     = cell_index <= NUM_OF_CHESS_PIECES;

            // place down chess pieces:
            switch (board_matrix_value)
            {
            default:                                                                              break;
            case rook:    create_chess_piece(board, cell_index, position, type, is_upper_board);  break;
            case knight:  create_chess_piece(board, cell_index, position, type, is_upper_board);  break;
            case bishop:  create_chess_piece(board, cell_index, position, type, is_upper_board);  break;
            case queen:   create_chess_piece(board, cell_index, position, type, is_upper_board);  break;
            case king:    create_chess_piece(board, cell_index, position, type, is_upper_board);  break;
            case pawn:    create_chess_piece(board, cell_index, position, type, is_upper_board);  break;
            }
        }
    }

    return board;
}

void board_destroy(board_t* board)
{
    for (size_t i = 0; i < BOARD_SZ; i++)
    {
        cell_destroy(board->cells[i]);
    }

    free(board);
}