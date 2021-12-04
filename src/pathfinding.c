#include "pathfinding.h"
#include "symbols.h"

void path_cast_ray(Map* map, Vec2 begin, Vec2 end, void(edit_tile)(Tile*))
{
    int x0 = begin.x, y0 = begin.y;
    int x1 = end.x, y1 = end.y;

    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    bool end_ray = false;
    for (;;)
    {
        if (end_ray)
            return;

        if (map_check_bounds(map, vec2(x0, y0)))
        {
            if (map_tile(map, vec2(x0, y0))->symbol == WALL)
                end_ray = true;

            edit_tile(map_tile(map, vec2(x0, y0)));
        }

        if (x0 == x1 && y0 == y1)
            break;

        e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

bool path_cast_ray_and_return(Map* map, Vec2 begin, Vec2 end, bool (*check)(Tile*))
{
    int x0 = begin.x, y0 = begin.y;
    int x1 = end.x, y1 = end.y;

    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    bool end_ray = false;
    for (;;)
    {
        if (end_ray)
            return false;

        if (map_check_bounds(map, vec2(x0, y0)))
        {
            if (map_tile(map, vec2(x0, y0))->symbol == WALL)
                end_ray = true;

            if (check(map_tile(map, vec2(x0, y0))))
                return true;
        }

        if (x0 == x1 && y0 == y1)
            break;

        e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
    return false;
}

static int distance(Vec2 begin, Vec2 end)
{
    return abs(begin.x - end.x) + abs(begin.y - end.y);
}

Vec2 path_find_next_move(Map* map, Vec2 begin, Vec2 end)
{
    typedef vec_t(Vec2) vec_pos_t;
    vec_pos_t candidates;
    vec_init(&candidates);

    vec_push(&candidates, vec2(begin.x - 1, begin.y));  // left
    vec_push(&candidates, vec2(begin.x + 1, begin.y));  // right
    vec_push(&candidates, vec2(begin.x, begin.y - 1));  // up
    vec_push(&candidates, vec2(begin.x, begin.y + 1));  // down

    Vec2 best_pos = vec2(-1, -1);
    int smallest_dist = 1000;
    for (int i = 0; i < candidates.length; i++)
    {
        if (!map_tile(map, candidates.data[i])->is_walkable)
            continue;

        int current_dist =
            distance(begin, candidates.data[i]) + distance(candidates.data[i], end);
        if (current_dist < smallest_dist)
        {
            smallest_dist = current_dist;
            best_pos = candidates.data[i];
        }
    }

    vec_deinit(&candidates);
    return vec2_sub(best_pos, begin);
}
