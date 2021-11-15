#include <ncurses.h>

#include <stdlib.h>
#include <assert.h>

#include "map.h"

static void dig_room(Map* map, Vec2 pos, Vec2 size)
{
    if (!map_check_bounds(pos) || !map_check_bounds(vec_add(pos, size)))
    {
        return;
    }

    for (int y = pos.y; y < pos.y + size.y; y++)
    {
        for (int x = pos.x; x < pos.x + size.x; x++)
        {
            // Skip the tiles that are already rooms or
            // the tiles that are near rooms.
            if (map->tiles[y][x].symbol == '.' ||
                map->tiles[y + 1][x + 1].symbol == '.' ||
                map->tiles[y + 1][x].symbol == '.' || map->tiles[y][x + 1].symbol == '.')
                continue;
            map->tiles[y][x].symbol = '.';
            map->tiles[y][x].is_walkable = true;
        }
    }
}

// Digs a corridor from `room2` to `room1`.
static void dig_corridor(Map* map, Vec2 room1_pos, Vec2 room1_size, Vec2 room2_pos,
                         Vec2 room2_size)
{
}

void map_draw(Map* map)
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            mvaddch(y, x, map->tiles[y][x].symbol);
        }
    }
}

Map* map_generate(Vec2* rogue_start_pos)
{
    Map* map = malloc(sizeof(Map));

    // Fill the whole map with walls.
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            map->tiles[y][x].pos = vec2(x, y);
            map->tiles[y][x].symbol = '#';
            map->tiles[y][x].is_walkable = false;
        }
    }

    // Dig rooms randomly.
    int num_rooms = (rand() % 20) + 15;
    int room_to_spawn_rogue = rand() % (num_rooms - 1);
    Vec2 prev_room_pos = vec2(0, 0);
    Vec2 prev_room_size = vec2(0, 0);
    for (int i = 0; i < num_rooms; i++)
    {
        Vec2 pos = vec2((rand() % (MAP_WIDTH - 20)) + 1, (rand() % (MAP_HEIGHT - 6)) + 1);
        Vec2 size = vec2((rand() % 19) + 4, (rand() % 5) + 3);
        dig_room(map, pos, size);
        dig_corridor(map, pos, size, prev_room_pos, prev_room_size);
        prev_room_pos = pos;
        prev_room_size = size;

        if (i == room_to_spawn_rogue)
        {
            *rogue_start_pos = vec2(pos.x + (size.x / 2), pos.y + (size.y / 2));
        }
    }

    return map;
}

void map_free(Map* map)
{
    free(map);
    map = NULL;
}

bool map_check_bounds(Vec2 pos)
{
    return pos.x >= 0 && pos.y >= 0 && pos.x < MAP_WIDTH && pos.y < MAP_HEIGHT;
}

bool map_is_walkable(Map* map, Vec2 pos)
{
    return map->tiles[pos.y][pos.x].is_walkable;
}
