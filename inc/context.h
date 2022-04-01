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
    int(*post_hook_render)();
}window_t;

window_t* window_new(unsigned int width, unsigned int height, const char* title, int(*post_hook_render)());

renderer_t* renderer_new();
int renderer_present();
void context_destroy();

#endif