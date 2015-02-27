// 2015 Adam Jesionowski

#include <stdlib.h>
#include "utils.h"
#include "rtos.h"

List_t* makeNode(void* owner)
{
	List_t* node = (List_t*)malloc(sizeof(List_t));
	node->next  = NULL;
	node->owner = owner;
	node->prev  = NULL;
	return node;
}

Task_t* makeTask(uintd_t timer, uint8_t prio)
{
	Task_t* task = (Task_t*)malloc(sizeof(Task_t));

	task->taskList.next  = NULL;
	task->taskList.owner = task;
	task->taskList.prev  = NULL;

	task->sleepTimer = timer;
	task->priority   = prio;

	return task;
}
