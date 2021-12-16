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
    int dmg;
} ItemWeapon;

typedef struct
{
    int defence;
} ItemArmor;

typedef vec_t(Item) vec_item_t;

void item_load_items();
Item item_spawn_item(char* name);

#endif  // ITEM_H
