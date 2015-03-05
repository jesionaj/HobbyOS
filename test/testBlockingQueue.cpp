// 2015 Adam Jesionowski

#include "CppUTest/TestHarness.h"
#include "queue.h"
#include "rtos.h"
#include "idleTask.h"
#include <iostream>

#define SIZE 25

TEST_GROUP(BlockingQueue)
{

    Queue_t queue;
    uint32_t data[SIZE];

    void setup()
    {
        RTOS_Initialize();
        StartTask(&idleTask);
        Tick();
        InitQueue(&queue, (uint8_t*)data, sizeof(uint32_t), SIZE);
    }

    void teardown()
    {

    }

    void CheckFrontAndCount(uintd_t expectedFront, uintd_t expectedCount)
    {
        LONGS_EQUAL(expectedFront, queue.front);
        LONGS_EQUAL(expectedCount, queue.count);
    }

    void CheckDataAt(uintd_t index, uint32_t expected)
    {
        LONGS_EQUAL(expected, data[index]);
    }

    void CheckAllData(uint32_t expected)
    {
        for(int i = 0; i < SIZE; i++)
        {
            CheckDataAt(i, expected);
        }
    }

    void CheckBlockedOnWrite(List_t* expected)
    {
        POINTERS_EQUAL(queue.tasksBlockedOnWrite, expected);
    }

    void CheckBlockedOnRead(List_t* expected)
    {
        POINTERS_EQUAL(queue.tasksBlockedOnRead, expected);
    }
};

/*
 * Check that the current task (idleTask in this case) is blocked to the queue on an empty dequeue
 */
TEST(BlockingQueue, BlockOnEmpty)
{
    uint32_t val = 0;

    DequeueBlocking(&queue, (uint8_t*)&val);

    CheckFrontAndCount(0, 0);
    CheckBlockedOnRead(&idleTask.taskList);
}

/*
 * Check that the current task (idleTask in this case) is blocked to the queue on a full enqueue
 */
TEST(BlockingQueue, BlockOnFull)
{
    uint32_t val = 0xDEADBEEF;

    for(int i = 0; i < 25; i++)
    {
        EnqueueBlocking(&queue, (uint8_t*)&val);
    }

    CheckFrontAndCount(0, 25);

    EnqueueBlocking(&queue, (uint8_t*)&val);

    CheckBlockedOnWrite(&idleTask.taskList);
}


// Below are tests from testQueue replicated with the blocking methods that don't run into a block

/*
 * Enqueue a single value
 */
TEST(BlockingQueue, EnqueueOne)
{
    uint32_t val = 0xDEADBEEF;
    EnqueueBlocking(&queue, (uint8_t*)&val);

    CheckFrontAndCount(0, 1);
    CheckDataAt(0, 0xDEADBEEF);
}

/*
 * Enqueue until the queue is full (but not over its limit)
 */
TEST(BlockingQueue, EnqueueFull)
{
    uint32_t val = 0xDEADBEEF;

    for(int i = 0; i < 25; i++)
    {
        EnqueueBlocking(&queue, (uint8_t*)&val);
    }

    CheckFrontAndCount(0, 25);
    CheckAllData(0xDEADBEEF);
}

/*
 * Insert and then dequeue a value
 */
TEST(BlockingQueue, DequeueOne)
{
    uint32_t insert = 0xDEADBEEF;
    uint32_t dequeue = 0;

    EnqueueBlocking(&queue, (uint8_t*)&insert);
    DequeueBlocking(&queue, (uint8_t*)&dequeue);

    CheckFrontAndCount(1, 0);
    LONGS_EQUAL(0xDEADBEEF, dequeue);
}

/*
 * Mix dequeue and enqueues, make sure the state is as expected each time
 */
TEST(BlockingQueue, DequeueEnqueueMix)
{
    // Enqueue 4, Dequeue 2, Enqueue 2, Dequeue 6
    // To do this, we're going to continually increment what we're inserting so we know that order is preserved

    uint32_t insert = 100;
    uint32_t dequeue = 0;

    for(int i = 0; i < 4; i++)
    {
        EnqueueBlocking(&queue, (uint8_t*)&insert);
        insert++;
    }

    CheckFrontAndCount(0, 4);

    for(int i = 0; i < 2; i++)
    {
        DequeueBlocking(&queue, (uint8_t*)&dequeue);
        LONGS_EQUAL(100+i, dequeue);
    }

    CheckFrontAndCount(2, 2);

    for(int i = 0; i < 2; i++)
    {
        EnqueueBlocking(&queue, (uint8_t*)&insert);
        insert++;
    }

    CheckFrontAndCount(2, 4);

    for(int i = 0; i < 4; i++)
    {
        DequeueBlocking(&queue, (uint8_t*)&dequeue);
        LONGS_EQUAL(102+i, dequeue);
    }

    CheckFrontAndCount(6, 0);
}

/*
 * Enqueue and dequeue enough to see that the queue circles around correctly
 */
TEST(BlockingQueue, CircularTest)
{
    uint32_t insert = 0xDEADBEEF;
    uint32_t dequeue = 0;

    for(int i = 0; i < 25; i++)
    {
        EnqueueBlocking(&queue, (uint8_t*)&insert);
    }

    CheckFrontAndCount(0, 25);

    for(int i = 0; i < 12; i++)
    {
        DequeueBlocking(&queue, (uint8_t*)&dequeue);
        LONGS_EQUAL(0xDEADBEEF, dequeue);
    }

    CheckFrontAndCount(12, 13);

    insert = 0xFEEDBEEF;
    for(int i = 0; i < 12; i++)
    {
        EnqueueBlocking(&queue, (uint8_t*)&insert);
    }

    CheckFrontAndCount(12, 25);

    for(int i = 0; i < 13; i++)
    {
        DequeueBlocking(&queue, (uint8_t*)&dequeue);
        LONGS_EQUAL(0xDEADBEEF, dequeue);
    }

    CheckFrontAndCount(0, 12);

    for(int i = 0; i < 12; i++)
    {
        DequeueBlocking(&queue, (uint8_t*)&dequeue);
        LONGS_EQUAL(0xFEEDBEEF, dequeue);
    }

    CheckFrontAndCount(12, 0);
}
