#include <libtcod.h>

#include "actor.h"

static bool check_collision(Map* map, Actor* actor, vec_actor_t* enemies, Vec2 dir)
{
    if (!map_check_bounds(map, actor->pos) || !map_is_walkable(map, actor->pos))
        return true;

    // TODO: the collisions with the player are not checked here. Fix it.
    for (int i = 0; i < enemies->length; ++i)
    {
        // We shouldn't check for collisions against the same actor.
        if (enemies->data[i].id != actor->id)
        {
            if (vec2_equals(enemies->data[i].pos, actor->pos))
                return true;
        }
    }

    return false;
}

void actor_free(Actor* actor)
{
    vec_deinit(&actor->name);
}

void actor_move(Map* map, vec_actor_t* enemies, Actor* actor, Vec2 dir)
{
    actor->pos = vec2_add(actor->pos, dir);

    if (check_collision(map, actor, enemies, dir))
    {
        actor->pos = vec2_sub(actor->pos, dir);
    }
}

void actor_attack(Actor* victim, Actor* attacker)
{
    victim->hp -= attacker->dmg;
    if (victim->hp <= 0)
        victim->is_alive = false;
}

void actor_on_event(Event* event)
{
    switch (event->type)
    {
        case EVENT_ATTACK:
        {
            EventAttack* event_attack = (EventAttack*)(event->data);
            actor_attack(event_attack->victim, event_attack->attacker);
            break;
        }
        default:
            break;
    }
}
