#ifndef ACTOR_H
#define ACTOR_H

#include "map.h"

typedef struct
{
    Vec2 pos;
    char symbol;
} Actor;

typedef vec_t(Actor) vec_actor_t;

void actor_move(Map* map, Actor* actor, Vec2 dir);

#endif  // ACTOR_H
