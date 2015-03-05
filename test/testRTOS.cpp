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

    Task_t* makeTask(uint8_t prio)
    {
        Task_t* task = (Task_t*)malloc(sizeof(Task_t));

        task->taskList.next  = NULL;
        task->taskList.owner = task;
        task->taskList.prev  = NULL;

        task->sleepTimer = 0;
        task->priority   = prio;

        return task;
    }

    void CheckCurrentTask(Task_t* expected)
    {
        POINTERS_EQUAL(expected, CurrentTask);
    }

    void CheckSleepingTasks(Task_t* expected)
    {
        if(expected == NULL)
        {
            POINTERS_EQUAL(expected, SleepingTasks);
        }
        else
        {
            CHECK_TRUE(IsNodeInList(&SleepingTasks, &expected->taskList));
        }
    }

    // We check the pointers instead of using IsNodeInList as order does matter for ready tasks
    void CheckReadyTaskFront(Task_t* expected, uintd_t priority)
    {
        if(expected == NULL)
        {
            POINTERS_EQUAL(expected, ReadyTasks[priority]);
        }
        else
        {
            POINTERS_EQUAL(&expected->taskList, ReadyTasks[priority]);
        }
    }

};

/*
 * Check that we've initialized as expected.
 */
TEST(RTOS, Initialized)
{
    int i;

    for(i = 0; i < NUM_PRIORITY_LEVELS; i++)
    {
        POINTERS_EQUAL(NULL, ReadyTasks[i]);
    }

    CheckCurrentTask(&idleTask);
}

/*
 * Start one task, doing nothing else.
 */
TEST(RTOS, StartSingleTask)
{
    Task_t* task = makeTask(PRIORITY_1);

    POINTERS_EQUAL(NULL, ReadyTasks[PRIORITY_1]);

    StartTask(task);

    CheckReadyTaskFront(task, PRIORITY_1);
    CheckCurrentTask(&idleTask);
    CheckSleepingTasks(NULL);
}

/*
 * Start a task and tick
 */
TEST(RTOS, StartTaskAndTick)
{
    Task_t* task = makeTask(PRIORITY_1);

    StartTask(task);
    Tick();

    CheckReadyTaskFront(NULL, PRIORITY_1);
    CheckCurrentTask(task);
    CheckSleepingTasks(NULL);
}

/*
 * Start a task, then delay it.
 */
TEST(RTOS, SleepTask)
{
    Task_t* task = makeTask(PRIORITY_1);

    StartTask(task);
    Tick();
    DelayCurrentTask(5);

    LONGS_EQUAL(5, task->sleepTimer);

    CheckReadyTaskFront(NULL, PRIORITY_1);
    CheckCurrentTask(&idleTask);
    CheckSleepingTasks(task);
}

/*
 * Start a task, delay it, then tick without waking.
 */
TEST(RTOS, SleepTaskAndTickBeforeWake)
{
    Task_t* task = makeTask(PRIORITY_1);

    StartTask(task);
    Tick();
    DelayCurrentTask(2);
    Tick();

    LONGS_EQUAL(1, task->sleepTimer);

    CheckReadyTaskFront(NULL, PRIORITY_1);
    CheckCurrentTask(&idleTask);
    CheckSleepingTasks(task);
}

/*
 * Start a task, delay it, then tick until it wakes.
 */
TEST(RTOS, SleepTaskAndTickReady)
{
    Task_t* task = makeTask(PRIORITY_1);

    StartTask(task);
    Tick();
    DelayCurrentTask(1);
    Tick();
    Tick();

    LONGS_EQUAL(0, task->sleepTimer);

    CheckReadyTaskFront(NULL, PRIORITY_1);
    CheckCurrentTask(task);
    CheckSleepingTasks(NULL);
}

/*
 * Start two tasks with different priorities
 */
TEST(RTOS, StartTwoTasks)
{
    Task_t* task1 = makeTask(PRIORITY_1);
    Task_t* task2 = makeTask(PRIORITY_2);

    StartTask(task1);
    StartTask(task2);

    CheckReadyTaskFront(task1, PRIORITY_1);
    CheckReadyTaskFront(task2, PRIORITY_2);
    CheckCurrentTask(&idleTask);
    CheckSleepingTasks(NULL);
}

/*
 * Start two tasks with the same priority
 */
TEST(RTOS, StartTwoTasksSamePrio)
{
    Task_t* task1 = makeTask(PRIORITY_1);
    Task_t* task2 = makeTask(PRIORITY_1);

    StartTask(task1);
    StartTask(task2);

    CheckReadyTaskFront(task2, PRIORITY_1);
    POINTERS_EQUAL(ReadyTasks[PRIORITY_1]->next, &task1->taskList);

    CheckCurrentTask(&idleTask);
    CheckSleepingTasks(NULL);
}

/*
 * Start two tasks with the same priority, then tick
 */
TEST(RTOS, TwoTasksSamePrioTick)
{
    Task_t* task1 = makeTask(PRIORITY_1);
    Task_t* task2 = makeTask(PRIORITY_1);

    StartTask(task1);
    StartTask(task2);
    Tick();

    // Task2 was added later, so it should be the one running
    CheckReadyTaskFront(task1, PRIORITY_1);
    CheckCurrentTask(task2);
    CheckSleepingTasks(NULL);
}

/*
 * Start two tasks, same priority, sleep one of them.
 */
TEST(RTOS, TwoTasksSamePrioTickThenSleep)
{
    Task_t* task1 = makeTask(PRIORITY_1);
    Task_t* task2 = makeTask(PRIORITY_1);

    StartTask(task1);
    StartTask(task2);
    Tick();

    // Task2 should be running
    CheckCurrentTask(task2);

    // Let's delay it
    DelayCurrentTask(2);

    // Now task1 should be running
    CheckReadyTaskFront(NULL, PRIORITY_1);
    CheckReadyTaskFront(NULL, PRIORITY_1);
    CheckCurrentTask(task1);
    CheckSleepingTasks(task2);
}

/*
 * Start two tasks, same priority, sleep one of them, tick until it wakes.
 */
TEST(RTOS, TwoTasksSamePrioTickSleepTick)
{
    Task_t* task1 = makeTask(PRIORITY_1);
    Task_t* task2 = makeTask(PRIORITY_1);

    StartTask(task1);
    StartTask(task2);
    Tick();
    DelayCurrentTask(1);
    Tick();
    Tick();

    // Task2 just woke up, so it should take priority
    CheckReadyTaskFront(task1, PRIORITY_1);
    CheckCurrentTask(task2);
    CheckSleepingTasks(NULL);
}

/*
 * Check that starting a higher priority task and ticking lets it take over
 */
TEST(RTOS, HighPriorityTakesPrecedence)
{
    Task_t* task1 = makeTask(PRIORITY_1);
    Task_t* task2 = makeTask(PRIORITY_1);

    CheckCurrentTask(&idleTask);

    StartTask(task1);
    Tick();

    CheckCurrentTask(task1);

    StartTask(task2);
    Tick();

    CheckCurrentTask(task2);
}

/*
 * Check that time-slicing works. Each tick should change which task is running
 */
TEST(RTOS, TimeSlicing)
{
    Task_t* task1 = makeTask(PRIORITY_1);
    Task_t* task2 = makeTask(PRIORITY_1);
    Task_t* task3 = makeTask(PRIORITY_1);

    StartTask(task1);
    StartTask(task2);
    StartTask(task3);

    Tick();
    CheckCurrentTask(task3);

    Tick();
    CheckCurrentTask(task2);

    Tick();
    CheckCurrentTask(task1);
}

/*
 * Test blocking the current running task to a list
 */
TEST(RTOS, BlockCurrentTaskToList)
{
    List_t* list = NULL;

    Task_t* task1 = makeTask(PRIORITY_1);
    StartTask(task1);
    Tick();

    CheckCurrentTask(task1);

    BlockCurrentTaskToList(&list);

    CheckCurrentTask(&idleTask);
    POINTERS_EQUAL(&task1->taskList, list);
}

/*
 * Start a bunch of tasks, block them, and test that ReadyEntireList works
 */
TEST(RTOS, ReadyTaskEntireList)
{
    List_t* list = NULL;

    Task_t* task1 = makeTask(PRIORITY_1);
    Task_t* task2 = makeTask(PRIORITY_2);
    Task_t* task3 = makeTask(PRIORITY_3);

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

    CheckReadyTaskFront(NULL, PRIORITY_1);
    CheckReadyTaskFront(NULL, PRIORITY_2);
    CheckReadyTaskFront(NULL, PRIORITY_3);

    ReadyTaskEntireList(&list);

    CheckReadyTaskFront(task1, PRIORITY_1);
    CheckReadyTaskFront(task2, PRIORITY_2);
    CheckReadyTaskFront(task3, PRIORITY_3);
}
