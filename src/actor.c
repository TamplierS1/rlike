#include <libtcod.h>

#include "actor.h"
#include "inventory.h"
#include "random.h"

static bool check_collision(Map* map, Actor* actor, vec_actor_t* enemies)
{
    if (!map_check_bounds(map, actor->pos) || !map_is_walkable(map, actor->pos))
        return true;

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
    sdsfree(actor->name);
}

void actor_move(Map* map, vec_actor_t* enemies, Actor* actor, Vec2 dir)
{
    actor->pos = vec2_add(actor->pos, dir);

    if (check_collision(map, actor, enemies))
    {
        actor->pos = vec2_sub(actor->pos, dir);
    }
}

void actor_attack(Actor* victim, Actor* attacker)
{
    if (rand_random_int(0, 100) > actor_get_accuracy(attacker))
        return;

    int total_dmg = 0;
    total_dmg += (int)(actor_get_phys_dmg(attacker) *
                       (1.0f - (actor_get_phys_res(victim) / 100.0f)));
    total_dmg += (int)(actor_get_fire_dmg(attacker) *
                       (1.0f - (actor_get_fire_res(victim) / 100.0)));
    total_dmg += (int)(actor_get_cold_dmg(attacker) *
                       (1.0f - (actor_get_cold_res(victim) / 100.0)));
    total_dmg += (int)(actor_get_lightning_dmg(attacker) *
                       (1 - (actor_get_lightning_res(victim) / 100.0)));
    total_dmg -= actor_get_defence(victim);

    victim->hp -= total_dmg;

    // TODO: move this check into `clear_dead_enemies` in game.c
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

ItemWeapon* actor_get_weapon(Actor* actor)
{
    Item* weapon = inv_find_item_ex(&actor->inventory, ITEM_WEAPON, true);
    if (weapon == NULL)
        return NULL;
    return ((ItemWeapon*)weapon->item);
}

ItemArmor* actor_get_armor(Actor* actor)
{
    Item* armor = inv_find_item_ex(&actor->inventory, ITEM_ARMOR, true);
    if (armor == NULL)
        return NULL;
    return ((ItemArmor*)armor->item);
}

int actor_get_phys_dmg(Actor* actor)
{
    ItemWeapon* weapon = actor_get_weapon(actor);
    return weapon == NULL ? 0 : weapon->physical_dmg;
}

int actor_get_fire_dmg(Actor* actor)
{
    ItemWeapon* weapon = actor_get_weapon(actor);
    return weapon == NULL ? 0 : weapon->fire_dmg;
}

int actor_get_cold_dmg(Actor* actor)
{
    ItemWeapon* weapon = actor_get_weapon(actor);
    return weapon == NULL ? 0 : weapon->cold_dmg;
}

int actor_get_lightning_dmg(Actor* actor)
{
    ItemWeapon* weapon = actor_get_weapon(actor);
    return weapon == NULL ? 0 : weapon->lightning_dmg;
}

int actor_get_accuracy(Actor* actor)
{
    ItemWeapon* weapon = actor_get_weapon(actor);
    return weapon == NULL ? 0 : weapon->accuracy;
}

int actor_get_defence(Actor* actor)
{
    ItemArmor* armor = actor_get_armor(actor);
    return armor == NULL ? 0 : armor->defence;
}

int actor_get_phys_res(Actor* actor)
{
    ItemArmor* armor = actor_get_armor(actor);
    return armor == NULL ? 0 : armor->physical_resistance;
}

int actor_get_fire_res(Actor* actor)
{
    ItemArmor* armor = actor_get_armor(actor);
    return armor == NULL ? 0 : armor->fire_resistance;
}

int actor_get_cold_res(Actor* actor)
{
    ItemArmor* armor = actor_get_armor(actor);
    return armor == NULL ? 0 : armor->cold_resistance;
}

int actor_get_lightning_res(Actor* actor)
{
    ItemArmor* armor = actor_get_armor(actor);
    return armor == NULL ? 0 : armor->lightning_resistance;
}
