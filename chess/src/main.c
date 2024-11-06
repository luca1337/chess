#define SDL_MAIN_HANDLED

#include <game.h>

int main(int argc, char **argv)
{
    game_t game;
    memset(&game, 0, sizeof(game_t));

    game_init(&game);
    game_update(&game);

    return 0;
}