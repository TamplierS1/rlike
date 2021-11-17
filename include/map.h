#ifndef MAP_H
#define MAP_H

#define MAP_WIDTH 80
#define MAP_HEIGHT 80

#include <stdbool.h>

#include "vec.h"
#include <libtcod.h>

#include "vec2.h"

#define ROOM_DENSITY 70
#define ROOM_SIZE_MIN vec2(7, 7)
#define ROOM_SIZE_MAX vec2(20, 20)

#define ENEMY_ROOMS 6

typedef struct
{
    Vec2 target;
    Vec2 offset;
} Camera;

typedef struct
{
    Vec2 pos;
    char symbol;
    bool is_walkable;
    bool is_visible;
    bool was_explored;
    TCOD_color_t back_color;
    TCOD_color_t fore_color;
} Tile;

typedef struct
{
    Vec2 pos;
    Vec2 size;
    Vec2 center;
} Room;

typedef vec_t(Room) vec_room_t;

typedef struct
{
    Tile tiles[MAP_HEIGHT][MAP_WIDTH];
    vec_room_t rooms;
} Map;

// I can't include `actor.h` (it would cause a cyclic dependency),
// so I have to use void* here.
Map* map_generate(Vec2* out_rogue_start_pos, void* out_enemies);
void map_free(Map* map);

void map_update_fog_of_war(Map* map, Vec2 player_pos, int player_vision_radius);
bool map_check_bounds(Vec2 pos);
bool map_is_walkable(Map* map, Vec2 pos);

#endif  // MAP_H
