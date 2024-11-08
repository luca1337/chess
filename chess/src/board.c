#include <board.h>
#include <cell.h>
#include <cglm/vec2.h>
#include <chess_piece.h>
#include <context.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Windows.h>

extern renderer_t *renderer;
extern window_t *window;

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
        chess_piece_t *current_piece = board->cells[chessIndex]->entity;

        if (current_piece != NULL)
        {
            current_piece->draw(current_piece);
        }
    }
}

static void place_chess_piece(board_t *board, unsigned index, vec2 position, piece_type_t type, char is_upper_board)
{
    chess_piece_t *piece = chess_piece_new(type, !is_upper_board, TRUE);
    piece->set_position(piece, (int)position[0], (int)position[1]);
    board->cells[index]->entity = piece;
    board->cells[index]->is_occupied = TRUE;
}

static void board_init(board_t *board)
{
    // Note that I didn't use FEN notation to place down cells.
    // it's something i might want to implement later so that
    // i can also connect a chess engine for the AI.

    color_t cell_color = color_create(0, 0, 0, 0);

    // place down board cells
    for (unsigned long columnIndex = 0ul; columnIndex != CELLS_PER_ROW; ++columnIndex)
    {
        const char is_odd = (columnIndex % 2);
        char is_black = is_odd;

        for (unsigned long rowIndex = 0ul; rowIndex != CELLS_PER_ROW; ++rowIndex)
        {
            // transform b-dim array to mono dimensional
            const int cell_index = (columnIndex * CELLS_PER_ROW) + rowIndex;

            const int pos_x = (cell_index % CELLS_PER_ROW) * CELL_SZ;
            const int pos_y = (cell_index / CELLS_PER_ROW) * CELL_SZ;

            vec2 position = {(float)pos_x, (float)pos_y};
            vec2 cell_size = {CELL_SZ, CELL_SZ};

            // swap color based on cell oddity/evenly
            cell_color = is_black ? BLACK : WHITE;
            is_black = !is_black;

            // after we calculated color of the cell and position, let's create it
            board->cells[cell_index] = cell_new(position, cell_size, cell_color);
            board->cells[cell_index]->pos_x = pos_x;
            board->cells[cell_index]->pos_y = pos_y;

            const int board_matrix_value = board_matrix[cell_index];
            const piece_type_t type = (piece_type_t)board_matrix_value;
            const char is_upper_board = (cell_index <= NUM_OF_CHESS_PIECES);

            // place down chess pieces for each of know type
            switch (board_matrix_value)
            {
            default: break;
            case rook: place_chess_piece(board, cell_index, position, type, is_upper_board); break;
            case knight: place_chess_piece(board, cell_index, position, type, is_upper_board); break;
            case bishop: place_chess_piece(board, cell_index, position, type, is_upper_board); break;
            case queen: place_chess_piece(board, cell_index, position, type, is_upper_board); break;
            case king: place_chess_piece(board, cell_index, position, type, is_upper_board); break;
            case pawn: place_chess_piece(board, cell_index, position, type, is_upper_board); break;
            }
        }
    }
}

void board_new(board_t *board)
{
    // board_t *board = (board_t *)calloc(1, sizeof(board_t));
    // CHECK(board, NULL, "Could not allocate memory for board");
    memset(board, 0, sizeof(board_t));

    board->draw = _draw_board;

    board_init(board);
}

void board_restore_state(board_t *board)
{
    for (unsigned long i = 0; i != BOARD_SZ; ++i)
    {
        cell_restore_state(board->cells[i]);
    }

    board_init(board);
}

void board_destroy(board_t *board)
{
    for (unsigned long i = 0; i != BOARD_SZ; ++i)
    {
        cell_destroy(board->cells[i]);
    }

    // free(board);
}