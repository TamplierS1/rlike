#ifndef ACTOR_H
#define ACTOR_H

#include "map.h"
#include "event.h"
#include "inventory.h"
#include "sds.h"

typedef struct
{
    int id;
    Vec2 pos;
    char symbol;
    TCOD_color_t color;
    sds name;

    int hp;
    int vision_radius;
    Inventory inventory;

    bool is_alive;
} Actor;

typedef struct
{
    Actor* attacker;
    Actor* victim;
    int player_id;
} EventAttack;

typedef struct
{
    Actor* dead_actor;
} EventDeath;

typedef vec_t(Actor) vec_actor_t;
typedef vec_t(Actor*) vec_actor_ptr_t;

void actor_free(Actor* actor);

void actor_move(Map* map, vec_actor_t* enemies, Actor* actor, Vec2 dir);
void actor_attack(Actor* victim, Actor* attacker);

void actor_on_event(Event* event);

#endif  // ACTOR_H
