#include "game.h"
#include "events.h"
#include "scoreboard.h"
#include "cell.h"

#include <stdlib.h>
#include <string.h>

// GLOBALS
window_t *window = NULL;
renderer_t *renderer = NULL;
events_t *events = NULL;
queue_t *texture_queue = NULL;

render_text_t *gameover_text = NULL;
texture_t *gameover_background = NULL;

int old_pos_x = 0;
int old_pos_y = 0;
int old_piece_cell_index = 0;

static void recycle_textures(chess_piece_t *piece)
{
    for (unsigned long i = 0ul; i < piece->moves_number; ++i)
    {
        if (!piece->moves || !piece->moves[i])
            continue;

        queue_enqueue(texture_queue, piece->moves[i]->markers);
    }
}

static void game_handle_pawn_promotion(game_t *game)
{
    if (!game->current_piece || game->current_piece->piece_type != pawn)
        return;

    piece_type_t types[] = {queen, knight, rook, bishop};

    if (game->current_piece->pos_y == 0 || game->current_piece->pos_y == (SCREEN_H - CELL_SZ))
    {
        for (unsigned long i = 0ul; i != PROMOTION_PIECES_COUNT; ++i)
        {
            int pos_y = game->current_piece->is_white ? ((game->current_piece->pos_y + CELL_SZ) + (CELL_SZ * i)) : ((game->current_piece->pos_y - CELL_SZ) - (CELL_SZ * i));
            chess_piece_t *promotion_piece = chess_piece_new(types[i], game->current_piece->is_white, FALSE);
            promotion_piece->set_position(promotion_piece, game->current_piece->pos_x, pos_y);
            game->promotion_pieces[i] = promotion_piece;
        }

        game->is_promoting_pawn = TRUE;
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

    if (!game->is_promoting_pawn)
    {
        // give a feedback to player of the current hovered cell
        color_t hightlight_color = game->current_player->is_white ? GREEN : RED;
        cell_highlight(game->board->cells[current_cell_index], mouse_x, mouse_y, hightlight_color);
    }

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
                // just ensure that the current player is the same as the clicked piece
                if (game->current_player->is_white == current_chess_piece->is_white)
                {
                    game->current_piece = current_chess_piece;
                    old_piece_cell_index = current_cell_index;
                    old_pos_x = game->current_piece->pos_x;
                    old_pos_y = game->current_piece->pos_y;

                    game->current_piece->generate_legal_moves(game->current_piece, game->board);

                    // check whether the king is in checkmate
                    if (game->current_piece->piece_type == king)
                    {
                        if (game->current_piece->moves_number == 0)
                        {
                            SDL_Log("King is in CHECKMATE, Game loss for: %s Team!", game->current_player->is_white ? "White" : "Black");

                            game->is_gameover = TRUE;
                        }
                    }
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
                // set the position to the new found cell
                game->current_piece->set_position(game->current_piece, found_cell->pos_x, found_cell->pos_y);

                if (found_cell->is_occupied)
                {
                    game->current_player->score += found_cell->entity->score_value;
                    scoreboard_update(&game->scoreboard, game->current_player);
                }

                // always check if a pawn can be promoted
                game_handle_pawn_promotion(game);

                chess_piece_set_entity_null(game->board, old_piece_cell_index);
                chess_piece_set_entity_cell(game->board, game->current_piece, current_cell_index);

                // Set current pawn to enpassant
                game->current_piece->is_enpassant = game->current_piece->piece_type == pawn && game->current_piece->is_first_move && abs(old_pos_y - (int)game->current_piece->pos_y) > CELL_SZ;

                // Set current king on castling
                game->current_piece->is_castling = game->current_piece->piece_type == king && game->current_piece->is_first_move && abs(old_pos_x - (int)game->current_piece->pos_x) > CELL_SZ;

                const char *player_color = game->current_player->is_white ? "White" : "Black";

#pragma region CASTLING

                enpassant_move_t ep_move;
                memset(&ep_move, 0, sizeof(enpassant_move_t));

                if (game->current_piece->piece_type == king)
                {
                    if (game->current_piece->is_castling)
                    {
                        // now swap rook and king's
                        const char is_long_castling = (current_cell_index < old_piece_cell_index);
                        SDL_Log(is_long_castling ? "[[LONG CASTLING]] of %s team" : "[[SHORT CASTLING]] of %s team", player_color);

                        // calculate correct indexes
                        const int left_rook_index = game->current_player->is_white ? 56 : 0; // indexes are hardcoded but we know by the board matrix that those are the same whenever the game start
                        const int right_rook_index = game->current_player->is_white ? 63 : 7;

                        // swap rook and king
                        ep_move.swap_index = is_long_castling ? current_cell_index + 1 : current_cell_index - 1;
                        ep_move.rook_index = is_long_castling ? left_rook_index : right_rook_index;
                        ep_move.target_rook = game->board->cells[ep_move.rook_index]->entity;
                        ep_move.swap_cell = game->board->cells[ep_move.swap_index];

                        // set rook new cell position
                        ep_move.target_rook->set_position(ep_move.target_rook, ep_move.swap_cell->pos_x, ep_move.swap_cell->pos_y);

                        // settle new/previous cells' state
                        chess_piece_set_entity_cell(game->board, ep_move.target_rook, ep_move.swap_index);
                        chess_piece_set_entity_null(game->board, ep_move.rook_index);
                    }
                }
#pragma endregion

#pragma region ENPASSANT
                if (game->current_piece->piece_type == pawn)
                {
                    int index = game->current_piece->is_white ? current_cell_index + CELLS_PER_ROW : current_cell_index - CELLS_PER_ROW;
                    chess_piece_t *enpassant_piece = game->board->cells[index]->entity;

                    if (enpassant_piece && enpassant_piece->is_enpassant)
                    {
                        chess_piece_set_entity_null(game->board, index);

                        SDL_Log("[[Enpassant Move]] from %s team!", player_color);

                        // update scoreboard
                        game->current_player->score += enpassant_piece->score_value;
                        scoreboard_update(&game->scoreboard, game->current_player);
                    }
                }
#pragma endregion

                game->current_piece->is_first_move = FALSE;

                if (!game->is_promoting_pawn)
                {
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
                    old_piece_cell_index = current_cell_index;
                }
            }
            else
            {
                game->current_piece->set_position(game->current_piece, old_pos_x, old_pos_y);
            }

            recycle_textures(game->current_piece);

            game->current_piece = NULL;
        }

        if (game->promoted_piece)
        {
            cell_t *found_cell = game->board->cells[old_piece_cell_index];

            game->promoted_piece->set_position(game->promoted_piece, found_cell->pos_x, found_cell->pos_y);

            chess_piece_set_entity_cell(game->board, game->promoted_piece, old_piece_cell_index);

            // swap player's turn and enqueue the old player to be ready for the next turn
            queue_enqueue(game->players_queue, game->current_player);
            game->current_player = queue_peek(game->players_queue);

            // update turn text
            text_update(game->player_turn_text, game->current_player->is_white ? "> WHITE'S TURN <" : "> BLACK'S TURN <");

            // dequeue old player
            queue_dequeue(game->players_queue);

            game->promoted_piece = NULL;
        }
    }
}

game_state_t *game_state_new()
{
    game_state_t *game = (game_state_t *)calloc(1, sizeof(game_state_t));
    CHECK(game, NULL, "Couldn't allocate memory for game_state struct");

    return game;
}

game_t *game_new()
{
    game_t *game = (game_t *)calloc(1, sizeof(game_t));
    CHECK(game, NULL, "Couldn't allocate memory for game struct");
    memset(game->game_states, 0, sizeof(game_state_t) * MAX_GAME_STATES);

    return game;
}

static void draw_legal_moves(game_t *game)
{
    if (is_mouse_button_down(events))
    {
        if (game->current_piece && game->current_piece->moves)
        {
            for (unsigned long i = 0ul; i < game->current_piece->moves_number; ++i)
            {
                game->current_piece->moves[i]->markers->render(game->current_piece->moves[i]->markers, SDL_ALPHA_OPAQUE / 2, NULL);
            }
        }
    }
}

static void draw_promotion_pieces(game_t *game)
{
    // draw pawn promotion textures
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    for (unsigned long i = 0ul; i < PROMOTION_PIECES_COUNT; i++)
    {
        if (game->promotion_pieces[i])
        {
            SDL_Log("Promoting pawn state");
            color_t color_mod = game->current_player->is_white ? GREEN : RED;
            SDL_SetTextureColorMod(game->promotion_pieces[i]->chess_texture->texture, color_mod.r, color_mod.g, color_mod.b);
            game->promotion_pieces[i]->draw(game->promotion_pieces[i]);

            // check if mouse is inside one of the available pieces to choose
            if ((mouse_x > game->promotion_pieces[i]->pos_x && (mouse_x < game->promotion_pieces[i]->pos_x + CELL_SZ)) && (mouse_y > game->promotion_pieces[i]->pos_y && mouse_y < (game->promotion_pieces[i]->pos_y + CELL_SZ)))
            {
                if (is_mouse_button_down(events))
                {
                    // promote pawn
                    game->promoted_piece = chess_piece_new(game->promotion_pieces[i]->piece_type, game->promotion_pieces[i]->is_white, TRUE);
                    game->is_promoting_pawn = FALSE;
                    memset(game->promotion_pieces, 0, sizeof(chess_piece_t) * PROMOTION_PIECES_COUNT);
                    break;
                }
            }
        }
    }
}

// SETUP STATE

void state_setup_enter(game_t *game)
{
    // create scoreboard
    scoreboard_new(&game->scoreboard);

    // create two players
    player_t *white_player = player_new(TRUE);
    player_t *black_player = player_new(FALSE);

    // enqueue the two players ans white starts
    game->players_queue = queue_new(MAX_PLAYERS, sizeof(player_t) * MAX_PLAYERS);
    queue_enqueue(game->players_queue, white_player);
    queue_enqueue(game->players_queue, black_player);

    game->current_player = queue_peek(game->players_queue);
    queue_dequeue(game->players_queue);
}

static game_state_t *state_setup_update(game_state_t *gs, game_t *game)
{
    return *gs->next;
}

void state_setup_exit(game_t *game)
{
}

// PLAY STATE

void state_play_enter(game_t *game)
{
}

game_state_t *state_play_update(game_state_t *gs, game_t *game)
{
    if (game->is_gameover)
    {
        return gs->next[1];
    }

    if (game->is_promoting_pawn)
    {
        return gs->next[0];
    }

    handle_chess_piece_selection(game);

    return gs;
}

void state_play_exit(game_t *game)
{
}

// PROMOTE PAWN STATE

void state_promote_pawn_enter(game_t *game)
{
}

game_state_t *state_promote_pawn_update(game_state_t *gs, game_t *game)
{
    draw_promotion_pieces(game);

    return game->is_promoting_pawn ? gs : gs->next[0];
}

void state_promote_pawn_exit(game_t *game)
{
}

// GAMEOVER STATE

void state_gameover_enter(game_t *game)
{
}

game_state_t *state_gameover_update(game_state_t *gs, game_t *game)
{
    // TODO: must implement a better way to manage game reset, right now we are alloc/dealloc memory continuously
    // and that will cause memory fragmentation, this is not what we want, we should pool memory instead. Right now
    // Whenever the game is ended, a new board is initialized by calling malloc on all cells/pieces/textures.

    if (window->keys[SDL_SCANCODE_SPACE])
    {
        gs->on_state_exit(game);
        gs->next[0]->on_state_enter(game);
        return gs->next[0];
    }

    gameover_background->render(gameover_background, 0, NULL);
    text_draw(gameover_text, (SCREEN_W / 2) - gameover_text->text_surface->w / 2, (SCREEN_H / 2) - CELL_SZ);

    return gs;
}

void state_gameover_exit(game_t *game)
{
    game->is_gameover = FALSE;
    game_reset_state(game);
}

void game_init(game_t *game)
{
    // create window, renderer and events
    window = window_new(SCREEN_W, SCREEN_H + CELL_SZ, "Chess-C");
    renderer = renderer_new(window);
    events = events_new();

    // Setup FSM
    game_state_t *state_setup = game_state_new();
    state_setup->on_state_enter = state_setup_enter;
    state_setup->on_state_update = state_setup_update;
    state_setup->on_state_exit = state_setup_exit;
    game->game_states[0] = state_setup;

    game_state_t *state_play = game_state_new();
    state_play->on_state_enter = state_play_enter;
    state_play->on_state_update = state_play_update;
    state_play->on_state_exit = state_play_exit;
    game->game_states[1] = state_play;

    game_state_t *state_promote = game_state_new();
    state_promote->on_state_enter = state_promote_pawn_enter;
    state_promote->on_state_update = state_promote_pawn_update;
    state_promote->on_state_exit = state_promote_pawn_exit;
    game->game_states[2] = state_promote;

    game_state_t *state_gameover = game_state_new();
    state_gameover->on_state_enter = state_gameover_enter;
    state_gameover->on_state_update = state_gameover_update;
    state_gameover->on_state_exit = state_gameover_exit;
    game->game_states[3] = state_gameover;

    // Link states
    state_setup->next[0] = state_play;
    state_play->next[0] = state_promote;
    state_play->next[1] = state_gameover;
    state_promote->next[0] = state_play;
    state_gameover->next[0] = state_setup;

    // Set current state
    game->current_state = state_setup;
    game->current_state->on_state_enter(game);

    // Create texture pool for later use
    texture_queue = queue_new(TEXTURE_POOL_SIZE, sizeof(texture_t) * TEXTURE_POOL_SIZE);
    for (unsigned long i = 0ul; i != TEXTURE_POOL_SIZE; ++i)
    {
        queue_enqueue(texture_queue, texture_load_from_file("../assets/textures/dot.png", TRUE));
    }

    // Creae board and pieces
    game->board = board_new();
    game->player_turn_text = text_new("../assets/fonts/Lato-Black.ttf", 14, "> WHITE'S TURN <", TURN);

    color_t gameover_background_color = color_create(0, 0, 0, 115);
    gameover_background = texture_create_raw(512, 512, gameover_background_color);

    color_t gameover_text_color = color_create(255, 255, 255, 0);
    gameover_text = text_new("../assets/fonts/Lato-Black.ttf", 28, "GAME LOSS!", gameover_text_color);
}

void game_reset_state(game_t* game)
{
    board_restore_state(game->board);
    scoreboard_reset_state(&game->scoreboard);
}

void game_update(game_t *game)
{
    while (renderer->is_running)
    {
        renderer_update_events_and_delta_time(window, renderer);

#pragma region RENDER OBJECTS
        game->board->draw(game->board);

        scoreboard_render(&game->scoreboard);

        text_draw(game->player_turn_text, (SCREEN_W / 2) - game->player_turn_text->width / 2, SCREEN_H + 14);

        draw_legal_moves(game);
#pragma endregion

        game->current_state = game->current_state->on_state_update(game->current_state, game);

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