#include "scoreboard.h"
#include "private.h"

#include <stdlib.h>
#include <string.h>

void scoreboard_new(scoreboard_t* scoreboard)
{
    memset(scoreboard, 0, sizeof(scoreboard_t));

    scoreboard->white_player_score = text_new("../assets/fonts/Lato-Black.ttf", 16, "WHITE SCORE: 0", WHITE);
    scoreboard->black_player_score = text_new("../assets/fonts/Lato-Black.ttf", 16, "BLACK SCORE: 0", BLACK);
}

void scoreboard_update(scoreboard_t* scoreboard, player_t* player)
{
    char update_white_score = player && player->is_white;

    const char* text = update_white_score ? "WHITE SCORE: %i" : "BLACK SCORE: %i";
    render_text_t* render_text = update_white_score ? scoreboard->white_player_score : scoreboard->black_player_score;

    char buffer[256];
    memset(buffer, 0, 256);
    sprintf_s(buffer, 256, text, player->score);
    text_update(render_text, buffer);
}

void scoreboard_render(scoreboard_t* scoreboard)
{
    text_draw(scoreboard->white_player_score, 0 + 15, SCREEN_H + 32);
    text_draw(scoreboard->black_player_score, SCREEN_W - scoreboard->black_player_score->width - 15, SCREEN_H + 32);
}

void scoreboard_reset_state(scoreboard_t* scoreboard)
{
    text_update(scoreboard->white_player_score, "WHITE SCORE: 0");
    text_update(scoreboard->black_player_score, "BLACK SCORE: 0");
}

void scoreboard_destroy(scoreboard_t* scoreboard)
{
    text_destroy(scoreboard->white_player_score);
    text_destroy(scoreboard->black_player_score);
}