// 2015 Adam Jesionowski

#include "config.h"
#include "list.h"
#include "task.h"
#include "rtos.h"
#include "config.h"
#include "idleTask.h"
#include "port.h"

// The following variables are mostly non-static as they're used by the testRTOS file.

// Lists that the RTOS handles
List_t* SleepingTasks;
List_t* BlockedTasks;
List_t* ReadyTasks[ NUM_PRIORITY_LEVELS ];

// Pointer to the current task
Task_t* CurrentTask;

// OS stack storage
static uintd_t OSStack[ OS_STACK_SIZE ];
volatile uintd_t* OSStackPtr = OSStack;

// We start at 1 as the first thing the RTOS does is decrement it using LOAD_REGISTERS
volatile uintd_t IntCount = 1;

// Pointer to current task's stack
volatile uintd_t* TaskStackPtr;

/*
 * Initialize RTOS variables and set idleTask as current task
 */
void RTOS_Initialize()
{
    int i;

    // This function occurs before interrupts are enabled
    IdleTask_init();

    SleepingTasks = NULL;
    BlockedTasks = NULL;

    for(i = 0; i < NUM_PRIORITY_LEVELS; i++)
    {
        ReadyTasks[i] = NULL;
    }

    CurrentTask = NULL;

#ifdef STACK_GROWS_TOWARD_ZERO
    // If the stack grows upwards, start at the end of the array
    OSStackPtr = &OSStack[DFLT_STACK_SIZE-1];
#endif
    InitStack(OSStackPtr, NULL);

    // The idle task will be the task that we start execution with
    TaskStackPtr = idleTask.stackPtr;
    CurrentTask  = &idleTask;
}

/*
 * Starts the passed task
 */
void StartTask(Task_t* task)
{
    ENTER_CRITICAL_SECTION;

    AppendToList(&ReadyTasks[task->priority], &task->taskList);

    EXIT_CRITICAL_SECTION;
}

/*
 * Suspends the current task
 */
void SuspendCurrentTask()
{
    ENTER_CRITICAL_SECTION;

    // So what's happening here is the current task's taskList is never moved to a ready list
    // As such, when we call SwitchToNextAvailableTask, the current task remains suspended
    // TODO: While this works, it's not exactly expected behavior. Should the Switch function change?
    SWITCH_TO_NEXT_INT; // Interrupts and calls SwitchToNextAvailableTask() from the OS stack

    EXIT_CRITICAL_SECTION;
}

/*
 * This function is called every millisecond. It handles switching which task is running.
 */
void Tick()
{
    uint8_t curPriority = 0;
    int8_t  i; // This needs to go under 0, hence signed.

    ENTER_CRITICAL_SECTION;

    // Update the stored task pointer to what it is now
    CurrentTask->stackPtr = TaskStackPtr;

    // Start by updating sleeping tasks
    UpdateSleeping();

    // Now, figure out who should take control.
    if(CurrentTask != NULL)
    {
        curPriority = CurrentTask->priority;
    }
    Task_t* nextTask = NULL;

    // Note that this will be true if there is a task with the same priority level waiting
    for(i = NUM_PRIORITY_LEVELS - 1; i >= curPriority; i--)
    {
        if(ReadyTasks[i] != NULL)
        {
            nextTask = (Task_t*)(ReadyTasks[i])->owner;
            break;
        }
    }
    
    if(nextTask != NULL)
    {
        RemoveFront(&ReadyTasks[nextTask->priority]);

        // We purposefully put the recently removed task at the end of the ready list to enable time-slicing
        // Putting it at the end gives each task equal share of processing
        if(CurrentTask != NULL)
        {
            AppendToEndOfList(&ReadyTasks[CurrentTask->priority], &CurrentTask->taskList);
        }
        CurrentTask = nextTask;

        // Update the task pointer to what it will be after we resume operation
        TaskStackPtr = nextTask->stackPtr;
    }

    // After this function returns, we load the registers of the CurrentTask using TaskStackPtr
    EXIT_CRITICAL_SECTION;
}

/*
 * This will cause the current task to sleep for the passed number of millisecond ticks.
 *
 * Note that this does not include the current tick, a full tick cycle needs to pass.
 * That is, if DelayCurrentTask(2) is called 0.5 ms after the last tick, the total time
 * the task waits will be 2.5 ms, 0.5 to finish the current cycle, then 2 ms to finish the two.
 *
 * For fine-grained timing, use timer.h instead.
 */
void DelayCurrentTask(uintd_t ticks)
{
    if(CurrentTask != NULL)
    {
    	ENTER_CRITICAL_SECTION;

    	// Set the sleep timer and add it to the sleeping tasks list
        CurrentTask->sleepTimer = ticks;
        AppendToList(&SleepingTasks, &CurrentTask->taskList);

        SWITCH_TO_NEXT_INT; // Interrupts and calls SwitchToNextAvailableTask() from the OS stack

        EXIT_CRITICAL_SECTION;
    }
}

/*
 * This adds the current task to a list (which should unblock it later), then switches to
 * another task. This means the task will not ready until it gets unblocked from the passed list.
 */
void BlockCurrentTaskToList(List_t** blockList)
{
    if(CurrentTask != NULL)
    {
    	ENTER_CRITICAL_SECTION;

        AppendToList(blockList, &CurrentTask->taskList);
        SWITCH_TO_NEXT_INT; // Interrupts and calls SwitchToNextAvailableTask() from the OS stack

        EXIT_CRITICAL_SECTION;
    }
}

/*
 * Loop through all the sleeping tasks and see if they can be readied.
 */
void UpdateSleeping()
{
    // This is called while in a critical section, so no interrupt protection here
    List_t* list = SleepingTasks;

    // For each task, check if the sleep timer is zero and ready it if so. If not, decrement timer count.
    while(list != NULL)
    {
        Task_t* task = (Task_t*)list->owner;
        List_t* next = list->next;

        if(task->sleepTimer == 0)
        {
            RemoveFromList(&SleepingTasks, list);
            AppendToList(&ReadyTasks[task->priority], list);
        }

        if(task->sleepTimer > 0)
        {
            task->sleepTimer--;
        }

        list = next;
    }
}

/*
 * This removes all tasks that are on the passed list and readies them
 * We do all of them rather than just the highest priority task to help prevent deadlocks
 */
void ReadyTaskEntireList(List_t** taskList)
{
    List_t* list = *taskList;

    ENTER_CRITICAL_SECTION;

    // Remove the task's list element from the passed list and add it to the ready list
    while(list != NULL)
    {
        Task_t* task = (Task_t*)list->owner;
        List_t* next = list->next;

        RemoveFromList(taskList, list);
        AppendToList(&ReadyTasks[task->priority], list);

        list = next;
    }

    EXIT_CRITICAL_SECTION;
}

// While the next two functions are similar, they are kept separate in case they
// change as they are doing fundamentally different things

/*
 * Change from the current task to the next available task. Called from the OS stack using SWITCH_TO_NEXT_INT.
 *
 * This is usually used immediately after a blocking or sleeping a task to let a new task take its place.
 */
void SwitchToNextAvailableTask()
{
    Task_t* nextTask = NULL;
    uintd_t i;

    ENTER_CRITICAL_SECTION;

    for(i = NUM_PRIORITY_LEVELS - 1; i >= 0; i--)
    {
        if(ReadyTasks[i] != NULL)
        {
            nextTask = (Task_t*)(ReadyTasks[i])->owner;
            break;
        }
    }

    // There should always be a next available task (namely, the idle task), but handle this anyway
    if(nextTask != NULL)
    {
    	//
        CurrentTask->stackPtr = TaskStackPtr;
        RemoveFront(&ReadyTasks[nextTask->priority]);

        CurrentTask = nextTask;
        TaskStackPtr = nextTask->stackPtr;
    }

    EXIT_CRITICAL_SECTION;
}

/*
 * Change from the current task to the next highest priority task from an ISR.
 *
 * This is useful to immediately kick off an important task.
 * Let's say you have an interrupt for a button press, and you want a task to occur
 * ASAP after that button has been pressed. Readying the task (by means of an event, for example)
 * then calling SwitchToHighestPriorityTaskFromISR() will allow that to happen, assuming the task
 * you want to occur would be the next highest priority task.
 */
void SwitchToHighestPriorityTaskFromISR()
{
    Task_t* nextTask = NULL;
    uint8_t curPriority = 0;
    uint8_t i;

    ENTER_CRITICAL_SECTION;

    // We're only interested in higher priority tasks, we don't want to interrupt the current one if
    // no high priority tasks are waiting
    if(CurrentTask != NULL)
    {
        curPriority = CurrentTask->priority;
    }

    for(i = NUM_PRIORITY_LEVELS - 1; i >= curPriority; i--)
    {
        if(ReadyTasks[i] != NULL)
        {
            nextTask = (Task_t*)(ReadyTasks[i])->owner;
            break;
        }
    }

    if(nextTask != NULL)
    {
        CurrentTask->stackPtr = TaskStackPtr;
        RemoveFront(&ReadyTasks[nextTask->priority]);

        AppendToEndOfList(&ReadyTasks[CurrentTask->priority], &CurrentTask->taskList);

        CurrentTask = nextTask;
        TaskStackPtr = nextTask->stackPtr;
    }

    EXIT_CRITICAL_SECTION;
}
