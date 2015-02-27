// 2015 Adam Jesionowski

/*
 * A queue implementation which stores arbitrary data.
 *
 * As we are in embedded environment, queues do not use malloc, but rather need storage
 * allocated for it. For example, for a queue that wants 6 elements of uint32_t:
 *
 * #define  QUEUE_SIZE 6
 * uint32_t  qStorage[QUEUE_SIZE];
 * &qStorage is then passed as the start argument in InitQueue
 *
 * There are two types of queue operations, blocking and non-blocking. Non-blocking calls
 * return a value indicating whether the operation was successful or not, depending on whether
 * there was room for new data/data available for enqueue and dequeue respectively.
 *
 * Blocking operations cause the task calling the function to wait until there is room for
 * data or for data to be available. Note that timeouts for these functions are currently not
 * implemented, so care should be taken when using them.
 *
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include "config.h"
#include "list.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct _queue_t
{
    uint8_t* start;                 // A pointer to the storage area

    uintd_t  count;                 // The current number of elements stored in the queue
    uintd_t  maxSize;               // The maximum number of elements that can be stored in the queue
    uintd_t  front;                 // The front of the queue (represented as an integer position in the queue, not a pointer)
    uintd_t  sizeOf;                // The size in bytes of the queue's content

    List_t*  tasksBlockedOnRead;    // A list of tasks that are waiting for data that they can dequeue
    List_t*  tasksBlockedOnWrite;   // A list of tasks that are waiting for space to enqueue data
} Queue_t;

void InitQueue(Queue_t* queue, uint8_t* start, uintd_t sizeOf, uintd_t maxSize);
bool Enqueue(Queue_t* queue, uint8_t* src);
bool Dequeue(Queue_t* queue, uint8_t* dest);
void EnqueueBlocking(Queue_t* queue, uint8_t* src);
void DequeueBlocking(Queue_t* queue, uint8_t* dest);
bool QueueIsEmpty(Queue_t* queue);
bool QueueIsFull(Queue_t* queue);

#ifdef	__cplusplus
}
#endif

#endif /* QUEUE_H_ */
