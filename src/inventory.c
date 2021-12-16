#include "sds.h"

#include "inventory.h"
#include "map.h"
#include "actor.h"

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
    // Check if any of the items are equipped.
    for (int i = 0; i < inv->items.length; i++)
    {
        if (inv->items.data[i].equipped && inv->items.data[i].category == item->category)
            return;
    }

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

void inv_remove_item(Inventory* inv, int idx)
{
    if (inv->items.length > idx)
        vec_swapsplice(&inv->items, idx, 1);
}

void inv_on_event(Event* event)
{
    switch (event->type)
    {
        case EVENT_DEATH:
        {
            EventDeath* event_death = ((EventDeath*)event->data);
            Map* map = event_death->map;

            if (event_death->dead_actor->inventory.items.length == 0)
                break;

            // TODO: its not random. It needs to be.
            int idx = 0;
            Item random_item = event_death->dead_actor->inventory.items.data[idx];
            inv_add_item(&map_tile(map, event_death->dead_actor->pos)->items,
                         &random_item);
            break;
        }
        default:
            break;
    }
}
