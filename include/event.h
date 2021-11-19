#ifndef EVENT_H
#define EVENT_H

typedef enum
{
    // `data` contains a vector of collided actors.
    // The first actor is a victim of an attack.
    // The second actor is the attacker.
    // Check `actor_on_event` for an example.
    EVENT_ATTACK,
    // 'data' contains the dead actor.
    EVENT_DEATH,
} EventType;

typedef struct
{
    EventType type;
    void* data;
} Event;

typedef void(*Subscriber)(Event* event);

void event_system_init();
void event_subscribe(Subscriber sub);
// Events don't typically live after this function's call,
// so don't store them.
void event_send(Event* event);
void event_unsubscribe(Subscriber sub);
void event_system_deinit();

#endif  // EVENT_H
