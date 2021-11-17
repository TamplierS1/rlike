#ifndef ACTOR_H
#define ACTOR_H

#include "map.h"
#include "event.h"

typedef struct
{
    int id;
    Vec2 pos;
    char symbol;

    int hp;
    int dmg;

    bool is_alive;
} Actor;

typedef vec_t(Actor) vec_actor_t;
typedef vec_t(Actor*) vec_actor_ptr_t;

void actor_move(Map* map, vec_actor_t* enemies, Actor* actor, Vec2 dir);
void actor_attack(Actor* victim, Actor* attacker);

void actor_on_event(Event* event);

#endif  // ACTOR_H
