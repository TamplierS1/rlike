#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>
#include "sds.h"
#include "vec.h"

typedef enum
{
    ITEM_WEAPON,
    ITEM_ARMOR,
    ITEM_EMPTY,
} ItemCategory;

typedef enum
{
    ITEM_PREFIX_FINE,
    ITEM_PREFIX_NONE
} ItemPrefix;

typedef struct
{
    sds name;
    ItemCategory category;
    void* item;
    bool equipped;
    int id;
} Item;

typedef struct
{
    int physical_dmg;
    int fire_dmg;
    int cold_dmg;
    int lightning_dmg;
} ItemWeapon;

typedef struct
{
    int defence;
    // Resistances are represented as %
    int physical_resistance;
    int fire_resistance;
    int cold_resistance;
    int lightning_resistance;
} ItemArmor;

typedef vec_t(Item) vec_item_t;

void item_init();
Item item_spawn_item(sds name, int depth);
void item_end();

#endif  // ITEM_H
