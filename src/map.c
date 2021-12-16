#include <stdlib.h>
#include <stdio.h>

#include "libtcod/color.h"
#include "mt19937ar.h"

#include "map.h"
#include "sds.h"
#include "serialization.h"
#include "actor.h"
#include "pathfinding.h"
#include "inventory.h"
#include "vec.h"

// clang-format off
#define FLOOR_BACK_COLOR (TCOD_color_t){35, 35, 35}
#define FLOOR_FORE_COLOR (TCOD_color_t){0, 0, 0}
#define WALL_BACK_COLOR (TCOD_color_t){0, 0, 0}
#define WALL_FORE_COLOR (TCOD_color_t){79, 73, 67}
// clang-format on

static int rand_int(int min, int max)
{
    // This is to avoid deleting by 0.
    min++;
    max++;
    // genrand_int32 generates up to a certain number exclusively.
    // So to include that number into generation we add one to max.
    max++;

    if (min < 0)
    {
        min -= min;
        max -= min;
    }

    int result = genrand_int32() % max;
    if (result < min)
        return min - 1;
    else if (min < 0)
        return result + min - 1;
    else
        return result - 1;
}

static bool is_area_available(Map* map, Vec2 begin, Vec2 end)
{
    if (!map_check_bounds(map, end))
        return false;

    for (int y = begin.y; y < end.y; y++)
    {
        for (int x = begin.x; x < end.x; x++)
        {
            if (map_tile(map, vec2(x, y))->symbol != map->wall_char)
            {
                return false;
            }
        }
    }

    return true;
}

static vec_actor_t* enemy_templates()
{
    static vec_actor_t vec;
    return &vec;
}

static Actor* find_enemy_template(sds name)
{
    vec_actor_t* temps = enemy_templates();
    for (int i = 0; i < temps->length; i++)
    {
        if (memcmp(temps->data[i].name, name, sdslen(temps->data[i].name)) == 0)
            return &temps->data[i];
    }
    return NULL;
}

static void spawn_enemy(sds name, Vec2 pos, vec_actor_t* out_enemies)
{
    static int enemy_id = 1;

    Actor* template = find_enemy_template(name);
    if (template == NULL)
    {
        error(__FILE__, __func__, __LINE__, "failed to spawn enemy %s at (%d, %d)", name,
              pos.x, pos.y);
        return;
    }

    Inventory inv = inv_create_inventory();
    for (int i = 0; i < template->inventory.items.length; i++)
    {
        Item item = item_spawn_item(template->inventory.items.data[i].name);
        item.equipped = template->inventory.items.data[i].equipped;
        inv_add_item(&inv, &item);
        inv_equip_item(&inv, item.id);
    }

    Actor enemy = {enemy_id++,   pos,          template->symbol,        template->color,
                   sdsdup(name), template->hp, template->vision_radius, inv,
                   true};
    vec_push(out_enemies, enemy);
}

static void spawn_enemies(Map* map, vec_actor_t* out_enemies, Vec2 player_start_pos)
{
    int num_spawned = 0;
    for (int i = 0; i < map->rooms.length; i++)
    {
        // Don't spawn enemies with the player.
        if (vec2_equals(map->rooms.data[i].center, player_start_pos))
            continue;

        int num_to_spawn =
            rand_int(map->num_enemies_each_room_min, map->num_enemies_each_room_max);

        Room room = map->rooms.data[i];
        for (int j = 0; j < num_to_spawn; j++)
        {
            Vec2 pos = vec2(
                room.center.x + rand_int(-(room.size.x / 2 - 1), room.size.x / 2 - 1),
                room.center.y + rand_int(-(room.size.y / 2 - 1), room.size.y / 2 - 1));
            int enemy_to_spawn = rand_int(0, enemy_templates()->length - 1);

            spawn_enemy(enemy_templates()->data[enemy_to_spawn].name, pos, out_enemies);
            num_spawned++;
        }
    }
    map->num_enemies = num_spawned;
}

static void fill_map_with_walls(Map* map)
{
    for (int y = 0; y < map->size.y; y++)
    {
        for (int x = 0; x < map->size.x; x++)
        {
            Tile tile = (Tile){
                .pos = vec2(x, y),
                .symbol = map->wall_char,
                .is_walkable = false,
                .is_visible = false,
                .was_explored = false,
                .back_color = WALL_BACK_COLOR,
                .fore_color = WALL_FORE_COLOR,
                .items = inv_create_inventory(),
            };
            vec_push(&map->tiles, tile);
        }
    }
}

static void dig(Map* map, Vec2 pos)
{
    *map_tile(map, pos) = (Tile){
        .pos = pos,
        .symbol = map->floor_char,
        .is_walkable = true,
        .is_visible = false,
        .was_explored = false,
        .back_color = FLOOR_BACK_COLOR,
        .fore_color = FLOOR_FORE_COLOR,
        .items = inv_create_inventory(),
    };
}

static bool dig_room(Map* map, Vec2 pos, Vec2 size)
{
    if (!map_check_bounds(map, pos) || !map_check_bounds(map, vec2_add(pos, size)))
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

        if (map_check_bounds(map, temp))
        {
            dig(map, temp);
        }
    }
}

// Returns the rogue's spawn position.
static Vec2 dig_rooms(Map* map)
{
    Vec2 spawn_pos = vec2(0, 0);
    for (int i = 0; i < map->room_density; i++)
    {
        Vec2 pos = vec2(rand_int(1, map->size.x), rand_int(1, map->size.y));
        Vec2 size = vec2(rand_int(map->room_size_min.x, map->room_size_max.x),
                         rand_int(map->room_size_min.y, map->room_size_max.y));

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

static void place_exit(Map* map, Vec2 player_pos)
{
    // Don't spawn in the same room with player.
    Room* exit = &map->rooms.data[rand_int(0, map->rooms.length - 1)];
    // The player always spawns in the center of the room.
    while (vec2_equals(exit->center, player_pos))
    {
        exit = &map->rooms.data[rand_int(0, map->rooms.length - 1)];
    }

    Vec2 pos =
        vec2(exit->center.x + rand_int(-(exit->size.x / 2 - 1), exit->size.x / 2 - 1),
             exit->center.y + rand_int(-(exit->size.y / 2 - 1), exit->size.y / 2 - 1));
    map->exit_pos = pos;

    map_tile(map, pos)->symbol = map->exit_char;
    map_tile(map, pos)->fore_color = TCOD_dark_grey;
}

void map_init()
{
    vec_init(enemy_templates());
    srz_load_enemy_templates("res/enemies", enemy_templates());
}

Map* map_generate(Vec2* out_player_start_pos, void* out_enemies)
{
    Map* map = malloc(sizeof(Map));
    vec_init(&map->rooms);
    vec_init(&map->tiles);
    map->exit_pos = vec2(-1, -1);

    map->size = vec2(80, 80);

    map->room_density = 70;
    map->room_size_min = vec2(7, 7);
    map->room_size_max = vec2(20, 20);

    map->num_enemies_each_room_min = 1;
    map->num_enemies_each_room_max = 3;
    map->num_enemies = 0;

    map->wall_char = '#';
    map->floor_char = ' ';
    map->exit_char = '>';

    vec_reserve(&map->tiles, map->size.x * map->size.y);

    fill_map_with_walls(map);
    *out_player_start_pos = dig_rooms(map);
    spawn_enemies(map, (vec_actor_t*)out_enemies, *out_player_start_pos);
    place_exit(map, *out_player_start_pos);

    return map;
}

void map_end(Map* map)
{
    vec_deinit(enemy_templates());
    vec_deinit(&map->rooms);
    vec_deinit(&map->tiles);
    free(map);
    map = NULL;
}

static void make_visible(Tile* tile)
{
    tile->is_visible = true;
    tile->was_explored = true;
}

void map_update_fog_of_war(Map* map, Vec2 player_pos, int player_vision_radius)
{
    Vec2 pos = player_pos;
    int radius = player_vision_radius;

    // Make every cell in the visible radius invisible.
    for (int x = pos.x - radius - 1; x <= pos.x + radius + 1; ++x)
    {
        for (int y = pos.y - radius - 1; y <= pos.y + radius + 1; ++y)
        {
            if (!map_check_bounds(map, vec2(x, y)))
                continue;
            map_tile(map, vec2(x, y))->is_visible = false;
        }
    }

    // Cast rays to the upper and bottom part of visible radius.
    for (int x = pos.x - radius; x <= pos.x + radius; ++x)
    {
        path_cast_ray(map, pos, vec2(x, pos.y - radius), make_visible);
        path_cast_ray(map, pos, vec2(x, pos.y + radius), make_visible);
    }

    // Right and left part.
    for (int y = pos.y - radius; y <= pos.y + radius; ++y)
    {
        path_cast_ray(map, pos, vec2(pos.x + radius, y), make_visible);
        path_cast_ray(map, pos, vec2(pos.x - radius, y), make_visible);
    }
}

bool map_check_bounds(Map* map, Vec2 pos)
{
    return pos.x > 0 && pos.y > 0 && pos.x < map->size.x && pos.y < map->size.y;
}

bool map_is_walkable(Map* map, Vec2 pos)
{
    return map_tile(map, pos)->is_walkable;
}

Tile* map_tile(Map* map, Vec2 pos)
{
    return &map->tiles.data[pos.x + pos.y * map->size.x];
}
