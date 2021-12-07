#ifndef INVENTORY_H
#define INVENTORY_H

#include "vec.h"

typedef enum
{
    ITEM_WEAPON
} ItemCategory;

typedef struct
{
    vec_char_t name;
    ItemCategory category;
    void* item;
} Item;

typedef struct
{
    int dmg;
} ItemWeapon;

typedef vec_t(Item) vec_item_t;

typedef struct
{
    vec_item_t items;
} Inventory;

Inventory inv_create_inventory();
void inv_construct_item(Inventory* inv, char* name, ItemCategory category, void* item);
void inv_add_item(Inventory* inv, Item* item);

#endif  // INVENTORY_H
