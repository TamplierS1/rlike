#include <ncurses.h>

#include <stdlib.h>

#include "map.h"
#include "symbols.h"
#include "actor.h"

static int rand_int(int min, int max)
{
    int result = rand() % max;
    if (result < min)
        return min;
    else
        return result;
}

static bool is_area_available(Map* map, Vec2 begin, Vec2 end)
{
    if (!map_check_bounds(end))
        return false;

    for (int y = begin.y; y < end.y; y++)
    {
        for (int x = begin.x; x < end.x; x++)
        {
            if (map->tiles[y][x].symbol != WALL)
            {
                return false;
            }
        }
    }

    return true;
}

static void dig(Map* map, Vec2 pos)
{
    map->tiles[pos.y][pos.x].symbol = FLOOR;
    map->tiles[pos.y][pos.x].is_walkable = true;
}

static bool dig_room(Map* map, Vec2 pos, Vec2 size)
{
    if (!map_check_bounds(pos) || !map_check_bounds(vec2_add(pos, size)))
    {
        return false;
    }

    // A room's position is at its upper-left corner.
    // So we +1 the position to not dig this corner.
    for (int y = pos.y + 1; y < pos.y + size.y; y++)
    {
        for (int x = pos.x + 1; x < pos.x + size.x; x++)
        {
            dig(map, vec2(x, y));
        }
    }

    return true;
}

static void spawn_enemies(Map* map, vec_actor_t* enemies)
{
    static int enemy_id = 1;

    int enemies_spawned = 0;
    int j;
    Room room;
    vec_foreach(&map->rooms, room, j)
    {
        if (enemies_spawned >= ENEMY_ROOMS)
            return;

        Actor enemy = {enemy_id++, room.center, ENEMY, 30, 5, true};
        vec_push(enemies, enemy);
        enemies_spawned++;
    }
}

static void fill_map_with_walls(Map* map)
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            map->tiles[y][x].pos = vec2(x, y);
            map->tiles[y][x].symbol = WALL;
            map->tiles[y][x].is_walkable = false;
        }
    }
}

static void connect_rooms(Map* map, Vec2 center1, Vec2 center2)
{
    Vec2 temp;
    temp.x = center1.x;
    temp.y = center1.y;

    while (true)
    {
        if (abs((temp.x - 1) - center2.x) < abs(temp.x - center2.x))
            temp.x--;
        else if (abs((temp.x + 1) - center2.x) < abs(temp.x - center2.x))
            temp.x++;
        else if (abs((temp.y + 1) - center2.y) < abs(temp.y - center2.y))
            temp.y++;
        else if (abs((temp.y - 1) - center2.y) < abs(temp.y - center2.y))
            temp.y--;
        else
            break;

        dig(map, temp);
    }
}

// Returns the rogue's spawn position.
static Vec2 dig_rooms(Map* map)
{
    Vec2 spawn_pos = vec2(0, 0);
    for (int i = 0; i < ROOM_DENSITY; i++)
    {
        Vec2 pos = vec2(rand_int(1, MAP_WIDTH), rand_int(1, MAP_HEIGHT));
        Vec2 size = vec2(rand_int(ROOM_SIZE_MIN.x, ROOM_SIZE_MAX.x),
                         rand_int(ROOM_SIZE_MIN.y, ROOM_SIZE_MAX.y));

        if (!is_area_available(map, pos, vec2_add(pos, vec2_add_int(size, 1))))
        {
            continue;
        }

        if (dig_room(map, pos, size))
        {
            Vec2 center = vec2(pos.x + size.x / 2, pos.y + size.y / 2);
            Room room = {pos, size, center};
            vec_push(&map->rooms, room);

            spawn_pos = center;

            if (i > 0)
            {
                connect_rooms(map, center, map->rooms.data[map->rooms.length - 2].center);
            }
        }
    }
    return spawn_pos;
}

Map* map_generate(Vec2* out_rogue_start_pos, void* out_enemies)
{
    Map* map = malloc(sizeof(Map));
    vec_init(&map->rooms);

    fill_map_with_walls(map);
    *out_rogue_start_pos = dig_rooms(map);
    spawn_enemies(map, (vec_actor_t*)out_enemies);

    return map;
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

void map_free(Map* map)
{
    vec_deinit(&map->rooms);
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
