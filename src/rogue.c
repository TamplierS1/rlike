#include "rogue.h"

void rogue_move(Map* map, Rogue* rogue, Vec2 dir)
{
    rogue->pos = vec_add(rogue->pos, dir);
    if (!map_check_bounds(rogue->pos) || !map_is_walkable(map, rogue->pos))
    {
        rogue->pos = vec_sub(rogue->pos, dir);
    }
}
