#ifndef MAP_H
#define MAP_H

#include <stdbool.h>

#include "vec.h"
#include <libtcod.h>

#include "vec2.h"

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
typedef vec_t(Tile) vec_tile_t;

typedef struct
{
    vec_tile_t tiles;
    vec_room_t rooms;
    Vec2 size;
    int room_density;
    Vec2 room_size_min;
    Vec2 room_size_max;
    int num_enemies;
} Map;

void map_init();
// I can't include `actor.h` (it would cause a cyclic dependency),
// so I have to use void* here.
Map* map_generate(Vec2* out_rogue_start_pos, void* out_enemies);
void map_end(Map* map);

void map_update_fog_of_war(Map* map, Vec2 player_pos, int player_vision_radius);
bool map_check_bounds(Map* map, Vec2 pos);
bool map_is_walkable(Map* map, Vec2 pos);
Tile* map_tile(Map* map, Vec2 pos);

#endif  // MAP_H
