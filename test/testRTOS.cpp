// 2015 Adam Jesionowski

#include <stdlib.h>
#include <iostream>
#include "CppUTest/TestHarness.h"
#include "rtos.h"
#include "task.h"
#include "utils.h"
#include "idleTask.h"

// These are declared in rtos.c.
extern Task_t* CurrentTask;
extern List_t* SleepingTasks;
extern List_t* ReadyTasks[ NUM_PRIORITY_LEVELS ];

TEST_GROUP(RTOS)
{



	void setup()
	{
		RTOS_Initialize();
	}
	void teardown()
	{

	}



};

TEST(RTOS, Initialized)
{
	int i;

	for(i = 0; i < NUM_PRIORITY_LEVELS; i++)
	{
		POINTERS_EQUAL(NULL, ReadyTasks[i]);
	}

	POINTERS_EQUAL(&idleTask, CurrentTask);
}

TEST(RTOS, AddTask)
{
	Task_t* task = makeTask(1, PRIORITY_1);

	POINTERS_EQUAL(NULL, ReadyTasks[PRIORITY_1]);

	StartTask(task);

	POINTERS_EQUAL(&task->taskList, ReadyTasks[PRIORITY_1]);
	POINTERS_EQUAL(&idleTask, CurrentTask);
	POINTERS_EQUAL(NULL, SleepingTasks);
}

TEST(RTOS, StartTaskAndTick)
{
	Task_t* task = makeTask(1, PRIORITY_1);

	StartTask(task);
	Tick();

	POINTERS_EQUAL(NULL, ReadyTasks[PRIORITY_1]);
	POINTERS_EQUAL(task, CurrentTask);
	POINTERS_EQUAL(NULL, SleepingTasks);
}

TEST(RTOS, SleepTask)
{
	Task_t* task = makeTask(1, PRIORITY_1);

	StartTask(task);
	Tick();
	DelayCurrentTask(5);

	LONGS_EQUAL(5, task->sleepTimer);
	POINTERS_EQUAL(NULL, ReadyTasks[PRIORITY_1]);
	POINTERS_EQUAL(&idleTask, CurrentTask);
	POINTERS_EQUAL(&task->taskList, SleepingTasks);
}

TEST(RTOS, SleepTaskAndTickBeforeWake)
{
	Task_t* task = makeTask(1, PRIORITY_1);

	StartTask(task);
	Tick();
	DelayCurrentTask(2);
	Tick();


	LONGS_EQUAL(1, task->sleepTimer);
	POINTERS_EQUAL(NULL, ReadyTasks[PRIORITY_1]);
	POINTERS_EQUAL(&idleTask, CurrentTask);
	POINTERS_EQUAL(&task->taskList, SleepingTasks);
}

TEST(RTOS, SleepTaskAndTickReady)
{
	Task_t* task = makeTask(1, PRIORITY_1);

	StartTask(task);
	Tick();
	DelayCurrentTask(1);
	Tick();
	Tick();


	LONGS_EQUAL(0, task->sleepTimer);
	POINTERS_EQUAL(NULL, ReadyTasks[PRIORITY_1]);
	POINTERS_EQUAL(task, CurrentTask);
	POINTERS_EQUAL(NULL, SleepingTasks);
}

TEST(RTOS, StartTwoTasks)
{
	Task_t* task1 = makeTask(1, PRIORITY_1);
	Task_t* task2 = makeTask(2, PRIORITY_2);

	StartTask(task1);
	StartTask(task2);

	POINTERS_EQUAL(&task1->taskList, ReadyTasks[PRIORITY_1]);
	POINTERS_EQUAL(&task2->taskList, ReadyTasks[PRIORITY_2]);
	POINTERS_EQUAL(&idleTask, CurrentTask);
	POINTERS_EQUAL(NULL, SleepingTasks);
}

TEST(RTOS, StartTwoTasksSamePrio)
{
	Task_t* task1 = makeTask(1, PRIORITY_1);
	Task_t* task2 = makeTask(2, PRIORITY_1);

	StartTask(task1);
	StartTask(task2);

	POINTERS_EQUAL(&task2->taskList, ReadyTasks[PRIORITY_1]);
	POINTERS_EQUAL(ReadyTasks[PRIORITY_1]->next, &task1->taskList);
	POINTERS_EQUAL(&idleTask, CurrentTask);
	POINTERS_EQUAL(NULL, SleepingTasks);
}

TEST(RTOS, TwoTasksSamePrioTick)
{
	Task_t* task1 = makeTask(1, PRIORITY_1);
	Task_t* task2 = makeTask(2, PRIORITY_1);

	StartTask(task1);
	StartTask(task2);
	Tick();

	POINTERS_EQUAL(&task1->taskList, ReadyTasks[PRIORITY_1]);
	POINTERS_EQUAL(task2, CurrentTask);
	POINTERS_EQUAL(NULL, SleepingTasks);
}

TEST(RTOS, TwoTasksSamePrioTickThenSleep)
{
	Task_t* task1 = makeTask(1, PRIORITY_1);
	Task_t* task2 = makeTask(2, PRIORITY_1);

	StartTask(task1);
	StartTask(task2);
	Tick();
	DelayCurrentTask(2);

	POINTERS_EQUAL(NULL, ReadyTasks[PRIORITY_1]);
	POINTERS_EQUAL(task1, CurrentTask);
	POINTERS_EQUAL(&task2->taskList, SleepingTasks);
}

TEST(RTOS, TwoTasksSamePrioTickSleepTick)
{
	Task_t* task1 = makeTask(1, PRIORITY_1);
	Task_t* task2 = makeTask(2, PRIORITY_1);

	StartTask(task1);
	StartTask(task2);
	Tick();
	DelayCurrentTask(2);
	Tick();

	POINTERS_EQUAL(NULL, ReadyTasks[PRIORITY_1]);
	POINTERS_EQUAL(task1, CurrentTask);
	POINTERS_EQUAL(&task2->taskList, SleepingTasks);
}

TEST(RTOS, HighPriorityTakesPrecedence)
{
	Task_t* task1 = makeTask(1, PRIORITY_1);
	Task_t* task2 = makeTask(2, PRIORITY_1);

	POINTERS_EQUAL(&idleTask, CurrentTask);

	StartTask(task1);
	Tick();

	POINTERS_EQUAL(task1, CurrentTask);

	StartTask(task2);
	Tick();

	POINTERS_EQUAL(task2, CurrentTask);
}

TEST(RTOS, TimeSlicing)
{
	Task_t* task1 = makeTask(1, PRIORITY_1);
	Task_t* task2 = makeTask(2, PRIORITY_1);
	Task_t* task3 = makeTask(3, PRIORITY_1);

	StartTask(task1);
	StartTask(task2);
	StartTask(task3);
	Tick();
	POINTERS_EQUAL(task3, CurrentTask);
	Tick();
	POINTERS_EQUAL(task2, CurrentTask);
	Tick();
	POINTERS_EQUAL(task1, CurrentTask);
}

TEST(RTOS, BlockCurrentTaskToList)
{
	List_t* list = NULL;

	Task_t* task1 = makeTask(1, PRIORITY_1);
	StartTask(task1);
	Tick();

	POINTERS_EQUAL(task1, CurrentTask);

	BlockCurrentTaskToList(&list);

	POINTERS_EQUAL(&idleTask, CurrentTask);
	POINTERS_EQUAL(&task1->taskList, list);
}

TEST(RTOS, ReadyTaskEntireList)
{
	List_t* list = NULL;

	Task_t* task1 = makeTask(1, PRIORITY_1);
	Task_t* task2 = makeTask(1, PRIORITY_2);
	Task_t* task3 = makeTask(1, PRIORITY_3);

	StartTask(task1);
	Tick();
	BlockCurrentTaskToList(&list);

	POINTERS_EQUAL(&task1->taskList, list);

	StartTask(task2);
	Tick();
	BlockCurrentTaskToList(&list);

	POINTERS_EQUAL(&task2->taskList, list);

	StartTask(task3);
	Tick();
	BlockCurrentTaskToList(&list);

	POINTERS_EQUAL(&task3->taskList, list);
	POINTERS_EQUAL(&task2->taskList, task3->taskList.next);

	POINTERS_EQUAL(NULL, ReadyTasks[PRIORITY_1]);
	POINTERS_EQUAL(NULL, ReadyTasks[PRIORITY_2]);
	POINTERS_EQUAL(NULL, ReadyTasks[PRIORITY_3]);

	ReadyTaskEntireList(&list);

	POINTERS_EQUAL(&task1->taskList, ReadyTasks[PRIORITY_1]);
	POINTERS_EQUAL(&task2->taskList, ReadyTasks[PRIORITY_2]);
	POINTERS_EQUAL(&task3->taskList, ReadyTasks[PRIORITY_3]);
}
