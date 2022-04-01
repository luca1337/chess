#include "game.h"
#include "events.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern events_t *events;
extern queue_t *texture_queue;

int old_pos_x = 0;
int old_pos_y = 0;
int old_piece_cell_index = 0;

static void recycle_textures(chess_piece_t* piece)
{
    for (size_t i = 0; i < piece->moves_number; i++)
    {
        if (!piece->moves || !piece->moves[i])
            continue;

        queue_enqueue(texture_queue, piece->moves[i]->markers);
    }
}

static cell_t *find_matching_cell(game_t *game, size_t cell_index)
{
    if (game->current_piece)
    {
        cell_t *current_cell = game->board->cells[cell_index];

        if (game->current_piece->moves)
        {
            for (size_t i = 0; i != game->current_piece->moves_number; ++i)
            {
                if (!memcmp(game->current_piece->moves[i]->possible_cells, current_cell, sizeof(cell_t)))
                {
                    return current_cell;
                }
            }
        }
    }

    return NULL;
}

static void select_cells(game_t *game)
{
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    int current_cell_index = ((mouse_y / BOARD_SZ) * CELLS_PER_ROW) + mouse_x / BOARD_SZ;

    if (is_mouse_button_down(events))
    {
        if (!game->current_piece)
        {
            // time to query cells within mouse cursor
            cell_t *current_cell = game->board->cells[current_cell_index];

            // get chess piece from cell
            chess_piece_t *current_chess_piece = (chess_piece_t *)current_cell->entity;

            if (current_chess_piece)
            {
                // se è presente controllo il colore del giocatore che sia lo stesso
                // delle pedine che sto selezionando per evitare di poter prendere
                // scacchi che non appartengono al giocatore corrente
                if (game->current_player->is_white == current_chess_piece->is_white)
                {
                    // arrivati quà siamo sicuri che la pedina sia presente all'interno
                    game->current_piece = current_chess_piece;
                    old_piece_cell_index = current_cell_index;
                    old_pos_x = game->current_piece->pos_x;
                    old_pos_y = game->current_piece->pos_y;

                    game->current_piece->moves = game->current_piece->get_moves(game->current_piece, game->board);
                }
            }
        }
        else
        {
            game->current_piece->set_position(game->current_piece, mouse_x - (CELL_SZ / 2), mouse_y - (CELL_SZ / 2));
        }
    }
    else
    {
        if (game->current_piece)
        {
            cell_t *found_cell = find_matching_cell(game, current_cell_index);

            if (found_cell)
            {
                if (found_cell->is_occupied)
                {
                    game->current_player->score++;
                }

                piece_set_entity_null(game->board, old_piece_cell_index);
                piece_set_entity_cell(game->board, game->current_piece, current_cell_index);

                game->current_piece->set_position(game->current_piece, found_cell->pos_x, found_cell->pos_y);

                game->current_piece->is_first_move = 0;

                queue_enqueue(game->players_queue, game->current_player);
                game->current_player = queue_peek(game->players_queue);
                queue_dequeue(game->players_queue);
            }
            else
            {
                game->current_piece->set_position(game->current_piece, old_pos_x, old_pos_y);
            }

            recycle_textures(game->current_piece);

            game->current_piece = NULL;
        }
    }
}

game_t *game_new()
{
    return (game_t *)calloc(1,sizeof(game_t));
}

void game_init(game_t *game)
{
    // la creazione della scacchiera implementa anche il setup delle pedine
    game->board = board_new();

    // creiamo due giocatori ed incodiamoli nella coda
    player_t *white_player = player_new(TRUE);
    player_t *black_player = player_new(FALSE);

    // coda di 2 giocatori (inizia il bianco)
    game->players_queue = queue_new(MAX_PLAYERS, sizeof(player_t) * MAX_PLAYERS);
    queue_enqueue(game->players_queue, white_player);
    queue_enqueue(game->players_queue, black_player);

    game->current_player = queue_peek(game->players_queue);
    queue_dequeue(game->players_queue);
}

void game_update(game_t *game)
{
    select_cells(game);

    // draw board and pieces
    game->board->draw(game->board);

    if (is_mouse_button_down(events))
    {
        if (game->current_piece)
        {
            if (game->current_piece->moves)
            {
                for (size_t i = 0; i < game->current_piece->moves_number; i++)
                {
                    if (!game->current_piece->moves[i])
                        continue;

                    game->current_piece->moves[i]->markers->render(game->current_piece->moves[i]->markers, TRUE, 115, NULL);
                }
            }
        }
    }
}