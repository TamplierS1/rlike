#ifndef LOG_H
#define LOG_H

#include "vec.h"
#include "sds.h"
#include "event.h"

typedef vec_t(sds) vec_sds_t;

typedef struct
{
    vec_sds_t messages;
} MessageLog;

void log_init();
void log_end();

MessageLog* log_get_log();
void log_new_msg(sds msg);

void log_on_event(Event* event);

#endif  // LOG_H
