#ifndef MAP_H
#define MAP_H

#define MAP_WIDTH 80
#define MAP_HEIGHT 40

#include <stdbool.h>

#include "vec.h"

typedef struct
{
    Vec2 pos;
    char symbol;
    bool is_walkable;
} Tile;

typedef struct
{
    Tile tiles[MAP_HEIGHT][MAP_WIDTH];
} Map;

Map* map_generate(Vec2* rogue_start_pos);
void map_draw(Map* map);
void map_free(Map* map);

bool map_check_bounds(Vec2 pos);
bool map_is_walkable(Map* map, Vec2 pos);

#endif  // MAP_H
