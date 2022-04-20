#include "events.h"

#include <stdlib.h>
#include <string.h>


events_t* events_new()
{
    events_t* ev = (events_t*)malloc(sizeof(events_t));
    memset(ev, 0, sizeof(events_t));
    return ev;
}

void update_events(events_t* ev, SDL_Event* native_event)
{
    switch (native_event->type)
    {
    case SDL_MOUSEBUTTONDOWN:
        ev->is_mouse_button_down = 1;
        ev->is_mouse_button_up = 0;
        break;
    case SDL_MOUSEBUTTONUP:
        ev->is_mouse_button_down = 0;
        ev->is_mouse_button_up = 1;
        break;
    default: break;
    }
}

char is_mouse_button_down(events_t* ev) { return ev->is_mouse_button_down; }

char is_mouse_button_up(events_t* ev) { return ev->is_mouse_button_up; }