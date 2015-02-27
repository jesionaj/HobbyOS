// 2015 Adam Jesionowski

/*
 * Here we define the task struct.
 *
 * The InitStack function is implemented in port.c, as it is hardware dependent.
 */

#ifndef TASK_H
#define	TASK_H

#include "config.h"
#include "list.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct _task_t {
    uintd_t   priority;             // The task's priority level, with 0 being the lowest
    List_t    taskList;             // This list element is used to place the task on ready/sleeping/blocked lists
    uintd_t   sleepTimer;           // Used for delaying the task with DelayCurrentTask
    volatile uintd_t*  stackPtr;   // Pointer to the task's stack
} Task_t;



#ifdef	__cplusplus
}
#endif

#endif	/* TASK_H */

