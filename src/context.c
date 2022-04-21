#include "context.h"
#include "private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <SDL_mixer.h>

static uint64_t start = 0;
static uint64_t end = 0;

window_t *window_new(unsigned int width, unsigned int height, const char *title)
{
    window_t *win = (window_t *)calloc(1, sizeof(window_t));
    CHECK(win, NULL, "Couldn't allocate memory for window struct");

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        SDL_Log("Couldn't initialize SDL: [%s]", SDL_GetError());
        return NULL;
    }

    win->sdl_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0x0);
    if (!win->sdl_window)
    {
        SDL_Log("Couldn't initialize SDL window: [%s]", SDL_GetError());
        return NULL;
    }

    SDL_Surface *window_icon = IMG_Load("../assets/textures/chess.comp");
    if (!window_icon) { SDL_Log("Couldn't load window icon: [%s]", SDL_GetError()); }
    SDL_SetWindowIcon((SDL_Window *)win->sdl_window, window_icon);

    win->width = width;
    win->height = height;

    if (TTF_Init() != 0)
    {
        SDL_Log("Couldn't initialize TTF Engine: [%s]", SDL_GetError());
        return NULL;
    }

    if (Mix_OpenAudio(44000, AUDIO_S32LSB, 2, 4096) != 0) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't init SDL_Mixer: %s", Mix_GetError()); }

    return win;
}

renderer_t *renderer_new(window_t *window)
{
    renderer_t *rend = (renderer_t *)calloc(1, sizeof(renderer_t));
    CHECK(rend, NULL, "Couldn't allocate memory for renderer struct");

    rend->sdl_renderer = SDL_CreateRenderer((SDL_Window *)window->sdl_window, -1, SDL_RENDERER_ACCELERATED);
    if (!rend->sdl_renderer)
    {
        SDL_Log("Couldn't initialize SDL renderer: [%s]", SDL_GetError());
        return NULL;
    }

    start = SDL_GetPerformanceFrequency();
    rend->is_running = 1;

    return rend;
}

void renderer_update_events_and_delta_time(window_t *window, renderer_t *renderer)
{
    // manage events better than this shit
    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        if (ev.type == SDL_QUIT) { renderer->is_running = 0; }
    }

    // clear screen and add alpha blending
    SDL_SetRenderDrawBlendMode((SDL_Renderer *)renderer->sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor((SDL_Renderer *)renderer->sdl_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear((SDL_Renderer *)renderer->sdl_renderer);

    // update delta timing
    start = end;
    end = SDL_GetPerformanceCounter();
    window->delta_time = (float)(((end - start) * 1000) / (float)SDL_GetPerformanceFrequency());

    window->keys = (Uint8 *)SDL_GetKeyboardState(NULL);
}

void renderer_present(renderer_t *renderer) { SDL_RenderPresent((SDL_Renderer *)renderer->sdl_renderer); }

void context_destroy(window_t *window, renderer_t *renderer)
{
    SDL_DestroyWindow((SDL_Window *)window->sdl_window);
    SDL_DestroyRenderer((SDL_Renderer *)renderer->sdl_renderer);
}