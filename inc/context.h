#ifndef CONTEXT_H
#define CONTEXT_H

typedef struct renderer{
    char is_running;
    void* sdl_renderer;
}renderer_t;

typedef struct window{
    unsigned int width;
    unsigned int height;
    char* title;
    float delta_time;
    void* sdl_window;
    unsigned char* keys;
}window_t;

window_t* window_new(unsigned int width, unsigned int height, const char* title);

renderer_t* renderer_new(window_t *window);
void renderer_present(renderer_t* renderer);
void renderer_update_events_and_delta_time(window_t* window, renderer_t* renderer);
void context_destroy(window_t* window, renderer_t* renderer);

#endif