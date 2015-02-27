// 2015 Adam Jesionowski

#include "event.h"
#include "rtos.h"

void WaitForEvent(Event_t* event)
{
    BlockCurrentTaskToList(&event->blockedTasks);
}

void TriggerEvent(Event_t* event)
{
    ReadyTaskEntireList(&event->blockedTasks);
}
