#include "inventory.h"
#include "sds.h"

Item* inv_find_item(Inventory* inv, int item_id)
{
    for (int i = 0; i < inv->items.length; i++)
    {
        if (inv->items.data[i].id == item_id)
            return &inv->items.data[i];
    }
    return NULL;
}

Item* inv_find_item_ex(Inventory* inv, ItemCategory category, bool is_equipped)
{
    for (int i = 0; i < inv->items.length; i++)
    {
        if (inv->items.data[i].category == category &&
            inv->items.data[i].equipped == is_equipped)
            return &inv->items.data[i];
    }
    return NULL;
}

void inv_equip_item(Inventory* inv, int item_id)
{
    Item* item = inv_find_item(inv, item_id);
    if (item != NULL)
        inv_find_item(inv, item_id)->equipped = true;
}

void inv_unequip_item(Inventory* inv, int item_id)
{
    Item* item = inv_find_item(inv, item_id);
    if (item != NULL)
        inv_find_item(inv, item_id)->equipped = false;
}

Inventory inv_create_inventory()
{
    vec_item_t items;
    vec_init(&items);
    Inventory inv = {items};
    return inv;
}

void inv_add_item(Inventory* inv, Item* item)
{
    if (inv_find_item(inv, item->id) != NULL)
        return;

    vec_push(&inv->items, *item);
}
