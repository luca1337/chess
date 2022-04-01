#define SDL_MAIN_HANDLED

#include "events.h"
#include "game.h"
#include "queue.h"

// GLOBALS
window_t *window = NULL;
renderer_t *renderer = NULL;
events_t *events = NULL;
queue_t *texture_queue = NULL;

game_t *game = NULL;

void init()
{
    game = game_new();
    game_init(game);

    texture_queue = queue_new(TEXTURE_POOL_SIZE, sizeof(texture_t) * TEXTURE_POOL_SIZE);

    for (size_t i = 0; i != TEXTURE_POOL_SIZE; ++i)
    {
        texture_t* to_add = texture_load_from_file("../assets/dot.png");
        queue_enqueue(texture_queue, to_add);
    }
}

static int game_loop()
{
    game_update(game);

    return 0;
}

int main(int argc, char **argv)
{
    // create window, renderer and events
    window      = window_new(SCREEN_W, SCREEN_H, "Chess-C", game_loop);
    renderer    = renderer_new();
    events      = events_new();

    init();

    renderer_present();

    return 0;
}