#include "game.h"
#include "events.h"
#include "scoreboard.h"
#include "cell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBALS
window_t *window            = NULL;
renderer_t *renderer        = NULL;
events_t *events            = NULL;
queue_t *texture_queue      = NULL;

int old_pos_x               = 0;
int old_pos_y               = 0;
int old_piece_cell_index    = 0;

static void recycle_textures(chess_piece_t *piece)
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
    if (game->current_piece && game->current_piece->moves)
    {
        cell_t *result = game->board->cells[cell_index];

        for (unsigned long i = 0ul; i != game->current_piece->moves_number; ++i)
        {
            if (!memcmp(game->current_piece->moves[i]->possible_cells, result, sizeof(cell_t)))
            {
                return result;
            }
        }
    }

    return NULL;
}

static void handle_chess_piece_selection(game_t *game)
{
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    if (mouse_y >= SCREEN_H)
        return;

    int current_cell_index = ((mouse_y / BOARD_SZ) * CELLS_PER_ROW) + mouse_x / BOARD_SZ;

    if (is_mouse_button_down(events))
    {
        if (!game->current_piece)
        {
            // time to query cells within mouse cursor
            cell_t *current_cell = game->board->cells[current_cell_index];

            // get chess piece from cell
            chess_piece_t *current_chess_piece = current_cell->entity;

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

                    game->current_piece->generate_legal_moves(game->current_piece, game->board);
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
                    scoreboard_update(&game->scoreboard, game->current_player);
                }

                chess_piece_set_entity_null(game->board, old_piece_cell_index);
                chess_piece_set_entity_cell(game->board, game->current_piece, current_cell_index);

                game->current_piece->set_position(game->current_piece, found_cell->pos_x, found_cell->pos_y);

                game->current_piece->is_first_move = 0;

                // swap player's turn and enqueue the old player to be ready for the next turn
                queue_enqueue(game->players_queue, game->current_player);
                game->current_player = queue_peek(game->players_queue);

                // update turn text
                text_update(game->player_turn_text, game->current_player->is_white ? "> WHITE'S TURN <" : "> BLACK'S TURN <");

                // dequeue old player
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
    game_t* game = (game_t*)calloc(1, sizeof(game_t));
    CHECK(game, NULL, "Couldn't allocate memory for game struct");

    return game;
}

void game_init(game_t *game)
{
    // create window, renderer and events
    window = window_new(SCREEN_W, SCREEN_H + CELL_SZ, "Chess-C");
    renderer = renderer_new(window);
    events = events_new();

    // Create texture pool for later use
    texture_queue = queue_new(TEXTURE_POOL_SIZE, sizeof(texture_t) * TEXTURE_POOL_SIZE);
    for (unsigned long i = 0ul; i != TEXTURE_POOL_SIZE; ++i)
    {
        queue_enqueue(texture_queue, texture_load_from_file("../assets/textures/dot.png"));
    }

    // la creazione della scacchiera implementa anche il setup delle pedine
    game->board = board_new();
    game->player_turn_text = text_new("../assets/fonts/Lato-Black.ttf", 14, "> WHITE'S TURN <", TURN);

    // create scoreboard
    scoreboard_new(&game->scoreboard);

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
    while (renderer->is_running)
    {
        renderer_update_events_and_delta_time(window, renderer);

        // --- DRAW OBJECTS ---

        handle_chess_piece_selection(game);

        // draw board and pieces
        game->board->draw(game->board);

        // draw scoreboard
        scoreboard_render(&game->scoreboard);

        // draw turn's text 
        text_draw(game->player_turn_text, 0, 0, (SCREEN_W / 2) - game->player_turn_text->width / 2, SCREEN_H + 14);

        // draw possible moves around cells
        if (is_mouse_button_down(events))
        {
            if (game->current_piece && game->current_piece->moves)
            {
                for (unsigned long i = 0ul; i < game->current_piece->moves_number; ++i)
                {
                    game->current_piece->moves[i]->markers->render(game->current_piece->moves[i]->markers, TRUE, SDL_ALPHA_OPAQUE / 2, NULL);
                }
            }
        }

        // --- END DRAW OBJECTS ---

        // present to screen
        renderer_present(renderer);
    }

    // destroy all resources and deallocate memory
    context_destroy(window, renderer);
    game_destroy(game);
    SDL_Quit();
    TTF_Quit();
}

void game_destroy(game_t *game)
{
    board_destroy(game->board);
    player_destroy(game->current_player);
    text_destroy(game->player_turn_text);
    scoreboard_destroy(&game->scoreboard);
}