// 2015 Adam Jesionowski

#include "config.h"
#include "queue.h"
#include "rtos.h"
#include "port.h"

/*
 * Initialize the queue struct
 */
void InitQueue(Queue_t* queue, uint8_t* start, uintd_t sizeOf, uintd_t maxSize)
{
    queue->start   = start;
    queue->front   = 0;
    queue->count   = 0;
    queue->maxSize = maxSize;
    queue->sizeOf  = sizeOf;
    queue->tasksBlockedOnRead  = NULL;
    queue->tasksBlockedOnWrite = NULL;
}

/*
 * The EnqueueOp and DequeueOp functions perform the bulk of the Enqueue/Dequeue work.
 *
 * Checking whether data is available/queue is not full is done by the blocking/non-blocking
 * functions that call these functions.
 */

/*
 * Add an element to the queue
 */
static void EnqueueOp(Queue_t* queue, uint8_t* src)
{
    uintd_t   i;
    uintd_t   pos;
    uint8_t*  tail;

    // pos represents where we are in the queue in terms of element count
    pos  = (queue->front + queue->count) % queue->maxSize;

    // tail is a pointer to where we are in the queue in terms of raw bytes
    tail = queue->start + (pos * queue->sizeOf);

    // Copy the data from src into the queue
    for(i = 0; i < queue->sizeOf; i++)
    {
        tail[i] = src[i];
    }

    // Increment the item count
    queue->count++;

    // If we have any tasks waiting for data to be added, unblock them.
    if(queue->tasksBlockedOnRead != NULL)
    {
        ReadyTaskEntireList(&queue->tasksBlockedOnRead);
    }
}

/*
 * Remove an element from the queue, copying it to dest
 */
static void DequeueOp(Queue_t* queue, uint8_t* dest)
{
    uintd_t i;
    uint8_t* head;

    head = queue->start + (queue->front * queue->sizeOf);

    for(i = 0; i < queue->sizeOf; i++)
    {
        dest[i] = head[i];
    }
    queue->count--;
    queue->front = (queue->front + 1) % queue->maxSize;

    if(queue->tasksBlockedOnWrite != NULL)
    {
        ReadyTaskEntireList(&queue->tasksBlockedOnWrite);
    }
}

/*
 * Non-blocking Enqueue operation. If there is room in the queue, add the new element.
 * Otherwise, return false.
 */
bool Enqueue(Queue_t* queue, uint8_t* src)
{
    bool error = true;

    ENTER_CRITICAL_SECTION;

    // Only do this if the count is less than the size of the queue
    if(queue->count < queue->maxSize)
    {
        EnqueueOp(queue, src);
        error = false;
    }

    EXIT_CRITICAL_SECTION;

    return error;
}

/*
 * Non-blocking Dequeue operation. If there is data in the queue, remove and return it.
 * Otherwise, return false.
 */
bool Dequeue(Queue_t* queue, uint8_t* dest)
{
    bool error = true;

    ENTER_CRITICAL_SECTION;

    // Only do this if there is any data
    if(queue->count != 0)
    {
        DequeueOp(queue, dest);
        error = false;
    }

    EXIT_CRITICAL_SECTION;

    return error;
}

/*
 * Blocking Enqueue operation. If there is no room in the queue, the task calling this function will block
 * until there is.
 */
void EnqueueBlocking(Queue_t* queue, uint8_t* src)
{
    bool wait = true;

    // In order to test this function, LOOP is used as a define in config.h
    // For running on the target hardware, LOOP(x) is defined as while(x).
    // On a host computer, it's defined as "", allowing us to test this function, as otherwise
    // it would sit in a loop, unable to return.
    LOOP(wait)
    {
        ENTER_CRITICAL_SECTION;

        if(queue->count < queue->maxSize)
        {
            wait = false;
            EnqueueOp(queue, src);
        }

        EXIT_CRITICAL_SECTION;

        // If we didn't add data, wait until we can.
        // Note that multiple tasks can be blocked on a single queue, and all tasks will be unblocked
        // immediately, even if there is only one space available. If this occurs, the highest priority task will
        // take the space, and the rest will re-block.
        if(wait)
        {
            BlockCurrentTaskToList(&queue->tasksBlockedOnWrite);
        }
    }
}

/*
 * Blocking Dequeue operation. If there is no data in the queue, the task calling this function will block
 * until there is.
 */
void DequeueBlocking(Queue_t* queue, uint8_t* dest)
{
    bool wait = true;

    LOOP(wait)
    {
        ENTER_CRITICAL_SECTION;

        if(queue->count != 0)
        {
            wait = false;
            DequeueOp(queue, dest);
        }

        EXIT_CRITICAL_SECTION;

        if(wait)
        {
            BlockCurrentTaskToList(&queue->tasksBlockedOnRead);
        }
    }
}

bool QueueIsEmpty(Queue_t* queue)
{
    return (queue->count == 0);
}

bool QueueIsFull(Queue_t* queue)
{
    return (queue->count >= queue->maxSize);
}

uintd_t QueueSize(Queue_t* queue)
{
    return queue->count;
}
