#ifndef MAP_H
#define MAP_H

#define MAP_WIDTH 80
#define MAP_HEIGHT 40

#include <stdbool.h>

#include "vec.h"

#include "vec2.h"

#define ROOM_DENSITY 70
#define ROOM_SIZE_MIN vec2(9, 5)
#define ROOM_SIZE_MAX vec2(19, 8)

typedef struct
{
    Vec2 pos;
    char symbol;
    bool is_walkable;
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

Map* map_generate(Vec2* rogue_start_pos);
void map_draw(Map* map);
void map_free(Map* map);

bool map_check_bounds(Vec2 pos);
bool map_is_walkable(Map* map, Vec2 pos);

#endif  // MAP_H
