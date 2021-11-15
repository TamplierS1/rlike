#include "actor.h"

void actor_move(Map* map, Actor* actor, Vec2 dir)
{
    actor->pos = vec_add(actor->pos, dir);
    if (!map_check_bounds(actor->pos) || !map_is_walkable(map, actor->pos))
    {
        actor->pos = vec_sub(actor->pos, dir);
    }
}
