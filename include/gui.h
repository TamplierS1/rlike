#ifndef GUI_H
#define GUI_H

#include <stdbool.h>

#include <SDL.h>

#include "actor.h"
#include "event.h"

void gui_on_event(Event* event);
bool gui_handle_input(SDL_Keysym key, Inventory* player_inv);
void gui_render(TCOD_Console* console, Actor* player, int depth);

#endif  // GUI_H
