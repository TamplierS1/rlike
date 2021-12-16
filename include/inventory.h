#ifndef INVENTORY_H
#define INVENTORY_H

#include "vec.h"

#include "item.h"
#include "event.h"

typedef struct
{
    vec_item_t items;
} Inventory;

Item* inv_find_item(Inventory* inv, int item_id);
Item* inv_find_item_ex(Inventory* inv, ItemCategory category, bool is_equipped);
void inv_equip_item(Inventory* inv, int item_id);
void inv_unequip_item(Inventory* inv, int item_id);

Inventory inv_create_inventory();
void inv_add_item(Inventory* inv, Item* item);
void inv_remove_item(Inventory* inv, int idx);

void inv_on_event(Event* event);

#endif  // INVENTORY_H
