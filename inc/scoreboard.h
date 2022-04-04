#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include "player.h"
#include "text.h"
#include "texture.h"

typedef struct scoreboard{ 
    render_text_t* white_player_score;
    render_text_t* black_player_score;
}scoreboard_t;

void scoreboard_new(scoreboard_t* scoreboard);
void scoreboard_update(scoreboard_t* scoreboard, player_t* player);
void scoreboard_render(scoreboard_t* scoreboard);
void scoreboard_destroy(scoreboard_t* scoreboard);

#endif