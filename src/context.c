#include "context.h"
#include "events.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>

static uint64_t start = 0;
static uint64_t end = 0;

extern renderer_t *renderer;
extern window_t *window;
extern events_t *events;

window_t *window_new(unsigned int width, unsigned int height, const char *title, int (*post_hook_render)())
{
    window_t *win = (window_t *)malloc(sizeof(window_t));
    memset(win, 0, sizeof(window_t));

    // todo: check for nullptr

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return NULL;
    }

    win->sdl_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0x0);
    if (!win->sdl_window)
    {
        printf("Couldn't initialize SDL window: [%s]\n", SDL_GetError());
        return NULL;
    }

    SDL_Surface* window_icon = IMG_Load("../assets/chess.png");
    if (!window_icon)
    {
        printf("Couldn't load window icon: [%s]\n", SDL_GetError());
    }
    SDL_SetWindowIcon((SDL_Window*)win->sdl_window, window_icon);

    win->width = width;
    win->height = height;
    win->post_hook_render = post_hook_render;

    return win;
}

renderer_t *renderer_new()
{
    renderer_t *renderer = (renderer_t *)malloc(sizeof(renderer_t));
    memset(renderer, 0, sizeof(renderer_t));

    // todo: check for nullptr

    renderer->sdl_renderer = SDL_CreateRenderer((SDL_Window *)window->sdl_window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if (!renderer->sdl_renderer)
    {
        printf("Couldn't initialize SDL renderer: [%s]\n", SDL_GetError());
        return NULL;
    }

    start = SDL_GetPerformanceFrequency();
    renderer->is_running = 1;

    return renderer;
}

int renderer_present()
{
    while (renderer->is_running)
    {
        SDL_PumpEvents();

        // manage events better than this shit
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT)
            {
                renderer->is_running = 0;
            }

            update_events(events, &ev);
        }

        // clear screen and add alpha blending
        SDL_SetRenderDrawBlendMode((SDL_Renderer *)renderer->sdl_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor((SDL_Renderer *)renderer->sdl_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear((SDL_Renderer *)renderer->sdl_renderer);

        // update delta timing
        end = SDL_GetPerformanceCounter();
        window->delta_time = (float)((end - start) * 1000 / (float)SDL_GetPerformanceFrequency());
        start = end;

        // draw
        window->post_hook_render();

        SDL_RenderPresent((SDL_Renderer *)renderer->sdl_renderer);
    }

    // dispose contexts
    context_destroy(window, renderer);

    return 0;
}

void context_destroy()
{
    SDL_DestroyWindow((SDL_Window *)window->sdl_window);
    SDL_DestroyRenderer((SDL_Renderer *)renderer->sdl_renderer);
    SDL_Quit();
}