// 2015 Adam Jesionowski

/*
 * This file handles task scheduling and switching.
 *
 * Every millisecond, the Tick function is called. This function will switch the
 * currently running task to the task with the highest priority.
 *
 * Tasks themselves can delay or block until a condition is met. If this occurs, the RTOS will
 * start running the next highest priority task.
 *
 * If the current task has the same priority as a waiting task or tasks and there are no other higher priority tasks,
 * the RTOS will switch to the first waiting task, and put the current task at the end of the waiting task list.
 * Time-slicing is implemented in this way.
 */

#ifndef RTOS_H_
#define RTOS_H_

#include "config.h"
#include "list.h"
#include "task.h"

#ifdef	__cplusplus
extern "C" {
#endif

void RTOS_Initialize();
void StartTask(Task_t* task);
void Tick();
void DelayCurrentTask(uintd_t ticks);
void UpdateSleeping();
void BlockCurrentTaskToList(List_t** blockList);
void ReadyTaskEntireList(List_t** taskList);
void SwitchToNextAvailableTask();
void SwitchToHighestPriorityTaskFromISR();

#ifdef	__cplusplus
}
#endif

#endif /* RTOS_H_ */
