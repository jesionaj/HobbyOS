// 2015 Adam Jesionowski

/*
 * Events are a way for tasks to wait for a certain trigger to occur.
 *
 * A task will call WaitForEvent and be blocked until the event producer
 * calls TriggerEvent. In this regard, they act like queues that don't pass data.
 */

#ifndef EVENT_H_
#define EVENT_H_

#include "list.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct _event_t
{
    List_t* blockedTasks;
} Event_t;

void WaitForEvent(Event_t* event);
void TriggerEvent(Event_t* event);

#ifdef	__cplusplus
}
#endif

#endif /* EVENT_H_ */
