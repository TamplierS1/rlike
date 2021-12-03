#ifndef GAME_H
#define GAME_H

#include <libtcod.h>

#include "map.h"
#include "actor.h"

typedef struct
{
    const char* title;
    int width;
    int height;
    int width_px;
    int height_px;

    TCOD_Console* console;
    TCOD_Tileset* tileset;
    TCOD_Context* context;
    bool quit;

    Map* map;
    // The player always has `id` 0.
    Actor player;
    vec_actor_t enemies;
    Camera camera;
    int player_vision_radius;
} Game;

void init(Game* game);
void update();
void end();

#endif  // GAME_H
