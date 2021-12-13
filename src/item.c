#include "serialization.h"
#include "vec.h"
#include "item.h"

static vec_item_t g_item_templates;

static Item spawn_empty_item(int id)
{
    return (Item){"EMPTY", ITEM_EMPTY, NULL, false, id};
}

static Item* find_template(sds name)
{
    for (int i = 0; i < g_item_templates.length; i++)
    {
        if (memcmp(g_item_templates.data[i].name, name,
                   sdslen(g_item_templates.data[i].name)) == 0)
            return &g_item_templates.data[i];
    }
    return NULL;
}

void item_load_items()
{
    if (srz_load_item_templates("res/items/weapons", &g_item_templates) != OK)
    {
        fatal(__FILE__, __func__, __LINE__, "failed to load item templates");
    }
}

Item item_spawn_item(sds name)
{
    static int id = 0;

    Item* template = find_template(name);
    if (template == NULL)
    {
        return spawn_empty_item(id++);
    }

    void* subitem;
    switch (template->category)
    {
        case ITEM_WEAPON:
            subitem = malloc(sizeof(ItemWeapon));
            break;
        default:
            fatal(__FILE__, __func__, __LINE__,
                  "not all item category options were covered.");
            return spawn_empty_item(id++);
    }
    memcpy(subitem, template->item, sizeof(subitem));

    Item item = {sdsdup(template->name), template->category, subitem, false, id};
    id++;

    return item;
}
