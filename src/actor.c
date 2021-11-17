#include <libtcod.h>

#include "actor.h"

// Returns the enemy the `actor` collided with.
// If there was no collisions returns NULL.
static Actor* check_collision(Actor* actor, vec_actor_t* enemies)
{
    int i;
    Actor* enemy;
    vec_foreach_ptr(enemies, enemy, i)
    {
        // We shouldn't check for collisions against the same actor.
        if (enemy->id != actor->id)
        {
            if (vec2_equals(enemy->pos, actor->pos))
            {
                return enemy;
            }
        }
    }

    return NULL;
}

void actor_move(Map* map, vec_actor_t* enemies, Actor* actor, Vec2 dir)
{
    actor->pos = vec2_add(actor->pos, dir);

    Actor* victim = check_collision(actor, enemies);
    if (victim != NULL)
    {
        vec_actor_ptr_t collided_actors;
        vec_init(&collided_actors);
        vec_push(&collided_actors, victim);
        vec_push(&collided_actors, actor);

        Event event = {EVENT_ATTACK, &collided_actors};
        event_send(&event);

        vec_deinit(&collided_actors);

        actor->pos = vec2_sub(actor->pos, dir);
    }

    if (!map_check_bounds(actor->pos) || !map_is_walkable(map, actor->pos))
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
            vec_actor_ptr_t* collided_actors = (vec_actor_ptr_t*)(event->data);
            Actor* victim = collided_actors->data[0];
            Actor* attacker = collided_actors->data[1];

            actor_attack(victim, attacker);
            break;
        }
    }
}
