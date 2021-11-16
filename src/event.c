#include "vec.h"

#include "event.h"

typedef vec_t(Subscriber) vec_sub_t;

static vec_sub_t* get_subs()
{
    static vec_sub_t subs;
    return &subs;
}

void event_system_init()
{
    vec_init(get_subs());
}

void event_subscribe(Subscriber sub)
{
    int idx;
    vec_find(get_subs(), sub, idx);
    // If the `sub` is already subscribed.
    if (idx != -1)
        return;

    vec_push(get_subs(), sub);
}

void event_send(Event* event)
{
    int i;
    Subscriber sub;
    vec_foreach(get_subs(), sub, i)
    {
        sub(event);
    }
}

void event_unsubscribe(Subscriber sub)
{
    // We don't need to check for the existence of sub
    // in our subs vector. If it's not there, then `vec_remove`
    // does nothing.
    vec_remove(get_subs(), sub);
}

void event_system_deinit()
{
    vec_deinit(get_subs());
}
