#ifndef EVENTS_H
#define EVENTS_H

#include <SDL.h>

typedef struct events {
    SDL_Event* native_event;
    char is_mouse_button_down;
    char is_mouse_button_up;
} events_t;

events_t* events_new();
void update_events(events_t* ev, SDL_Event* native_event);

char is_mouse_button_down(events_t* ev);
char is_mouse_button_up(events_t* ev);

#endif
