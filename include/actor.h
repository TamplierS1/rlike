#ifndef ACTOR_H
#define ACTOR_H

#include "map.h"
#include "event.h"
#include "inventory.h"
#include "sds.h"

// TODO: give enemies the depth they spawn at.
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
    Map* map;
} EventDeath;

typedef vec_t(Actor) vec_actor_t;
typedef vec_t(Actor*) vec_actor_ptr_t;

void actor_free(Actor* actor);

void actor_move(Map* map, vec_actor_t* enemies, Actor* actor, Vec2 dir);
void actor_attack(Actor* victim, Actor* attacker);

void actor_on_event(Event* event);

ItemWeapon* actor_get_weapon(Actor* actor);
ItemArmor* actor_get_armor(Actor* actor);

int actor_get_phys_dmg(Actor* actor);
int actor_get_fire_dmg(Actor* actor);
int actor_get_cold_dmg(Actor* actor);
int actor_get_lightning_dmg(Actor* actor);

int actor_get_defence(Actor* actor);
int actor_get_phys_res(Actor* actor);
int actor_get_fire_res(Actor* actor);
int actor_get_cold_res(Actor* actor);
int actor_get_lightning_res(Actor* actor);

#endif  // ACTOR_H
