#ifndef EVENT_H
#define EVENT_H

typedef enum
{
    EVENT_ATTACK,
    EVENT_DEATH,
} EventType;

typedef struct
{
    EventType type;
    void* data;
} Event;

typedef void (*Subscriber)(Event* event);

void event_system_init();
void event_subscribe(Subscriber sub);
// Events don't typically live after this function's call,
// so don't store them.
void event_send(Event* event);
void event_unsubscribe(Subscriber sub);
void event_system_deinit();

#endif  // EVENT_H
