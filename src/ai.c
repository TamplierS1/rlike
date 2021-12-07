#include "ai.h"
#include "game.h"
#include "pathfinding.h"

// TODO: come up with a better way to store these.
// indexes are the ids of the enemies
static Vec2 g_last_known_player_pos[64];
static bool g_has_seen_player[64];

static bool is_player_here(Tile* tile)
{
    return vec2_equals(tile->pos, find_player()->pos);
}

static bool look_for_player(Game* game, Actor* enemy)
{
    int radius = enemy->vision_radius;
    // Cast rays to the upper and bottom part of visible radius.
    for (int x = enemy->pos.x - radius; x <= enemy->pos.x + radius; ++x)
    {
        if (path_cast_ray_and_return(game->map, enemy->pos,
                                     vec2(x, enemy->pos.y - radius), is_player_here) ||
            path_cast_ray_and_return(game->map, enemy->pos,
                                     vec2(x, enemy->pos.y + radius), is_player_here))
        {
            return true;
        }
    }

    // Right and left part.
    for (int y = enemy->pos.y - radius; y <= enemy->pos.y + radius; ++y)
    {
        if (path_cast_ray_and_return(game->map, enemy->pos,
                                     vec2(enemy->pos.x + radius, y), is_player_here) ||
            path_cast_ray_and_return(game->map, enemy->pos,
                                     vec2(enemy->pos.x - radius, y), is_player_here))
        {
            return true;
        }
    }
    return false;
}

static void update_paths(Game* game)
{
    for (int i = 0; i < game->actors.length; i++)
    {
        Actor* enemy = &game->actors.data[i];
        if (enemy->id == game->player_id)
            continue;
        bool player_is_found = look_for_player(game, enemy);

        if (player_is_found)
        {
            Vec2 player_pos = find_player()->pos;
            g_last_known_player_pos[enemy->id] = player_pos;
            g_has_seen_player[enemy->id] = true;

            Vec2 dir = path_find_next_move(game->map, enemy->pos, player_pos);
            actor_move(game->map, &game->actors, enemy, dir);
        }
        else if (g_has_seen_player[enemy->id] &&
                 !vec2_equals(enemy->pos, g_last_known_player_pos[enemy->id]))
        {
            Vec2 dir = path_find_next_move(game->map, enemy->pos,
                                           g_last_known_player_pos[enemy->id]);
            actor_move(game->map, &game->actors, enemy, dir);
        }
    }
}

static void update_behaviour(Game* game)
{
    for (int i = 0; i < game->actors.length; i++)
    {
        Actor* enemy = &game->actors.data[i];
        if (enemy->id == game->player_id)
            continue;

        Vec2 dist_to_player = vec2_sub(find_player()->pos, enemy->pos);
        dist_to_player.x = abs(dist_to_player.x);
        dist_to_player.y = abs(dist_to_player.y);
        if (dist_to_player.x <= 1 && dist_to_player.y <= 1)
        {
            EventAttack attack = {enemy, find_player(), find_player()->id};
            Event event = {EVENT_ATTACK, &attack};
            event_send(&event);
        }
    }
}

void ai_init()
{
    for (int i = 0; i < 64; i++)
    {
        g_last_known_player_pos[i] = vec2(-1, -1);
        g_has_seen_player[i] = false;
    }
}

void ai_update(Game* game)
{
    update_paths(game);
    update_behaviour(game);
}
