#include "log.h"
#include "vec.h"
#include "actor.h"

static MessageLog g_log;
// When the log reaches the maximum capacity
// start deleting old messages.
static int g_max_capacity = 512;

static void clear_old_messages()
{
    int clearing_amount = 64;
    vec_splice(&g_log.messages, 0, clearing_amount);
}

void log_init()
{
    vec_init(&g_log.messages);
}

void log_end()
{
    for (int i = 0; i < g_log.messages.length; i++)
    {
        sdsfree(g_log.messages.data[i]);
    }
    vec_deinit(&g_log.messages);
}

MessageLog* log_get_log()
{
    return &g_log;
}

void log_new_msg(sds msg)
{
    if (g_log.messages.length >= g_max_capacity)
        clear_old_messages();
    vec_push(&g_log.messages, msg);
}

void log_on_event(Event* event)
{
    switch (event->type)
    {
        case EVENT_ATTACK:
        {
            EventAttack* event_attack = (EventAttack*)(event->data);

            Item* weapon =
                inv_find_item_ex(&event_attack->attacker->inventory, ITEM_WEAPON, true);
            if (weapon == NULL)
                break;

            log_new_msg(sdscatprintf(
                sdsempty(), "%s attacked %s for %d damage.", event_attack->attacker->name,
                event_attack->victim->name, ((ItemWeapon*)(weapon->item))->dmg));
            break;
        }
        case EVENT_DEATH:
        {
            EventDeath* event_death = (EventDeath*)(event->data);
            log_new_msg(
                sdscatprintf(sdsempty(), "%s died", event_death->dead_actor->name));
            break;
        }
    }
}
