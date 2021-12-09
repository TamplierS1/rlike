#ifndef INVENTORY_H
#define INVENTORY_H

#include <stdbool.h>

#include "vec.h"
#include "sds.h"

typedef enum
{
    ITEM_WEAPON
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

typedef vec_t(Item) vec_item_t;

typedef struct
{
    vec_item_t items;
} Inventory;

Item* inv_find_item(Inventory* inv, int item_id);
Item* inv_find_item_ex(Inventory* inv, ItemCategory category, bool is_equipped);
void inv_equip_item(Inventory* inv, int item_id);
void inv_unequip_item(Inventory* inv, int item_id);

Inventory inv_create_inventory();
Item inv_construct_item(char* name, ItemCategory category, void* subitem);
Item* inv_construct_add_item(Inventory* inv, char* name, ItemCategory category,
                             void* subitem);
void inv_add_item(Inventory* inv, Item* item);

#endif  // INVENTORY_H
