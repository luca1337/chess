#ifndef GAME_H
#define GAME_H

#include <board.h>
#include <chess_piece.h>
#include <player.h>
#include <queue.h>
#include <scoreboard.h>
#include <text.h>


typedef struct texture_pool {
    texture_t textures[TEXTURE_POOL_SIZE];
    int i, j;
} texture_pool_t;

typedef struct {
    int swap_index;
    int rook_index;
    chess_piece_t* target_rook;
    cell_t* swap_cell;
} enpassant_move_t;

typedef struct game_state game_state_t;
typedef struct game game_t;

// FSM
struct game_state {
    void (*on_state_enter)(game_t* game);
    game_state_t* (*on_state_update)(game_state_t* gs, game_t* game);
    void (*on_state_exit)(game_t* game);
    game_state_t* next[2]; // this should be a hashmap
};

game_state_t* game_state_new();

struct game {
    board_t board;
    queue_t* players_queue;
    player_t* current_player;
    chess_piece_t* current_piece;
    chess_piece_t* promoted_piece;
    render_text_t* player_turn_text;
    scoreboard_t scoreboard;
    chess_piece_t* promotion_pieces[PROMOTION_PIECES_COUNT];
    char is_promoting_pawn;

    // FSM
    game_state_t* game_states[MAX_GAME_STATES];
    game_state_t* current_state;
    char is_gameover;
};

game_t* game_new();
void game_init(game_t* game);
void game_reset_state(game_t* game);
void game_update(game_t* game);
void game_destroy(game_t* game);

#endif