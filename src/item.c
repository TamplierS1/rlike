#include "random.h"
#include "serialization.h"
#include "vec.h"
#include "item.h"

static vec_item_t g_templates;
// clang-format off
static int g_prefix_depths[] = {
    [ITEM_PREFIX_FINE] = 2,
    [ITEM_PREFIX_NONE] = 1
};
// clang-format on

static Item spawn_empty_item(int id)
{
    return (Item){"EMPTY", ITEM_EMPTY, NULL, false, id};
}

static Item* find_template(sds name)
{
    for (int i = 0; i < g_templates.length; i++)
    {
        if (memcmp(g_templates.data[i].name, name, sdslen(g_templates.data[i].name)) == 0)
            return &g_templates.data[i];
    }
    return NULL;
}

static ItemPrefix get_rand_prefix(int depth)
{
    vec_int_t possible_prefixes;
    vec_init(&possible_prefixes);

    for (int prefix = 0; prefix <= ITEM_PREFIX_NONE; prefix++)
    {
        if (g_prefix_depths[prefix] <= depth)
            vec_push(&possible_prefixes, prefix);
    }

    ItemPrefix prefix =
        (ItemPrefix)
            possible_prefixes.data[rand_random_int(0, possible_prefixes.length - 1)];

    vec_deinit(&possible_prefixes);

    return prefix;
}

static void apply_prefix(ItemCategory category, Item* item, ItemPrefix prefix)
{
    switch (prefix)
    {
        case ITEM_PREFIX_FINE:
            switch (category)
            {
                case ITEM_WEAPON:
                    ((ItemWeapon*)item->item)->dmg += 2;
                    break;
                case ITEM_ARMOR:
                    ((ItemArmor*)item->item)->defence += 1;
                    break;
                case ITEM_EMPTY:
                    break;
            }
            item->name = sdscat(sdsnew("Fine "), item->name);
            break;
        case ITEM_PREFIX_NONE:
            break;
    }
}

void item_init()
{
    if (srz_load_item_templates("res/items/weapons", &g_templates) != OK ||
        srz_load_item_templates("res/items/armor", &g_templates) != OK)
    {
        fatal(__FILE__, __func__, __LINE__, "failed to load item templates");
    }
}

Item item_spawn_item(sds name, int depth)
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
        case ITEM_ARMOR:
            subitem = malloc(sizeof(ItemArmor));
            break;
        default:
            fatal(__FILE__, __func__, __LINE__,
                  "not all item category options were covered.");
            return spawn_empty_item(id++);
    }
    memcpy(subitem, template->item, sizeof(subitem));

    Item item = {sdsdup(template->name), template->category, subitem, false, id};

    apply_prefix(template->category, &item, get_rand_prefix(depth));

    id++;
    return item;
}

void item_end()
{
    for (int j = 0; j < g_templates.length; j++)
    {
        free(g_templates.data[j].item);
        sdsfree(g_templates.data[j].name);
    }
    vec_deinit(&g_templates);
}
