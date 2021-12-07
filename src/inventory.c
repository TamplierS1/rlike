#include "inventory.h"

Inventory inv_create_inventory()
{
    vec_item_t items;
    vec_init(&items);
    Inventory inv = {items};
    return inv;
}

void inv_construct_item(Inventory* inv, char* name, ItemCategory category, void* item)
{
    vec_char_t name_v;
    vec_init(&name_v);
    vec_pusharr(&name_v, name, strlen(name));

    Item i = {name_v, category, item};

    vec_push(&inv->items, i);
}

void inv_add_item(Inventory* inv, Item* item)
{
    vec_push(&inv->items, *item);
}
