// 2015 Adam Jesionowski

#include "CppUTest/TestHarness.h"
#include "queue.h"
#include <iostream>

#define SIZE 25

TEST_GROUP(Queue)
{
    Queue_t queue;
    uint32_t data[SIZE];

    void setup()
    {
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
};

/*
 * Test that the Queue initializes correctly.
 */
TEST(Queue, InitQueue)
{
    // 25 uint32_ts
    POINTERS_EQUAL(data, queue.start);
    LONGS_EQUAL(0, queue.front);
    LONGS_EQUAL(0, queue.count);
    LONGS_EQUAL(SIZE, queue.maxSize);
    LONGS_EQUAL(sizeof(uint32_t), queue.sizeOf);
    POINTERS_EQUAL(NULL, queue.tasksBlockedOnRead);
    POINTERS_EQUAL(NULL, queue.tasksBlockedOnWrite);
}

/*
 * Enqueue a single value
 */
TEST(Queue, EnqueueOne)
{
    uint32_t val = 0xDEADBEEF;
    bool res = Enqueue(&queue, (uint8_t*)&val);

    CHECK_FALSE(res);
    CheckFrontAndCount(0, 1);
    CheckDataAt(0, 0xDEADBEEF);
}

/*
 * Enqueue until the queue is full (but not over its limit)
 */
TEST(Queue, EnqueueFull)
{
    uint32_t val = 0xDEADBEEF;

    for(int i = 0; i < 25; i++)
    {
        bool res = Enqueue(&queue, (uint8_t*)&val);
        CHECK_FALSE(res);
    }

    CheckFrontAndCount(0, 25);
    CheckAllData(0xDEADBEEF);
}

/*
 * Enqueue until the queue is full, then try adding one more.
 */
TEST(Queue, EnqueueOverFull)
{
    uint32_t val = 0xDEADBEEF;

    for(int i = 0; i < 25; i++)
    {
        bool res = Enqueue(&queue, (uint8_t*)&val);
        CHECK_FALSE(res);
    }

    val = 0xFEEDBEEF;
    bool res = Enqueue(&queue, (uint8_t*)&val);

    CHECK_TRUE(res);
    CheckFrontAndCount(0, 25);
    CheckAllData(0xDEADBEEF);
}

/*
 * Try to dequeue an empty queue.
 */
TEST(Queue, DequeueEmpty)
{
    uint32_t val = 0;
    bool res = Dequeue(&queue, (uint8_t*)&val);

    CHECK_TRUE(res);
    CheckFrontAndCount(0, 0);
}

/*
 * Insert and then dequeue a value
 */
TEST(Queue, DequeueOne)
{
    uint32_t insert = 0xDEADBEEF;
    uint32_t dequeue = 0;

    bool res = Enqueue(&queue, (uint8_t*)&insert);
    CHECK_FALSE(res);

    res = Dequeue(&queue, (uint8_t*)&dequeue);
    CHECK_FALSE(res);

    CheckFrontAndCount(1, 0);
    LONGS_EQUAL(0xDEADBEEF, dequeue);
}

/*
 * Mix dequeue and enqueues, make sure the state is as expected each time
 */
TEST(Queue, DequeueEnqueueMix)
{
    // Enqueue 4, Dequeue 2, Enqueue 2, Dequeue 6

    uint32_t insert = 100;
    uint32_t dequeue = 0;
    bool res;

    for(int i = 0; i < 4; i++)
    {
        res = Enqueue(&queue, (uint8_t*)&insert);
        CHECK_FALSE(res);
        insert++;
    }

    CheckFrontAndCount(0, 4);

    for(int i = 0; i < 2; i++)
    {
        res = Dequeue(&queue, (uint8_t*)&dequeue);
        CHECK_FALSE(res);
        LONGS_EQUAL(100 + i, dequeue);
    }

    CheckFrontAndCount(2, 2);

    for(int i = 0; i < 2; i++)
    {
        res = Enqueue(&queue, (uint8_t*)&insert);
        CHECK_FALSE(res);
        insert++;
    }

    CheckFrontAndCount(2, 4);

    for(int i = 0; i < 4; i++)
    {
        res = Dequeue(&queue, (uint8_t*)&dequeue);
        CHECK_FALSE(res);
        LONGS_EQUAL(102 + i, dequeue);
    }

    CheckFrontAndCount(6, 0);
}

/*
 * Enqueue and dequeue enough to see that the queue circles around correctly
 */
TEST(Queue, CircularTest)
{
    uint32_t insert = 0xDEADBEEF;
    uint32_t dequeue = 0;

    for(int i = 0; i < 25; i++)
    {
        bool res = Enqueue(&queue, (uint8_t*)&insert);
        CHECK_FALSE(res);
    }

    CheckFrontAndCount(0, 25);

    for(int i = 0; i < 12; i++)
    {
        bool res = Dequeue(&queue, (uint8_t*)&dequeue);
        CHECK_FALSE(res);
        LONGS_EQUAL(0xDEADBEEF, dequeue);
    }

    CheckFrontAndCount(12, 13);

    insert = 0xFEEDBEEF;
    for(int i = 0; i < 12; i++)
    {
        bool res = Enqueue(&queue, (uint8_t*)&insert);
        CHECK_FALSE(res);
    }

    CheckFrontAndCount(12, 25);

    for(int i = 0; i < 13; i++)
    {
        bool res = Dequeue(&queue, (uint8_t*)&dequeue);
        CHECK_FALSE(res);
        LONGS_EQUAL(0xDEADBEEF, dequeue);
    }

    CheckFrontAndCount(0, 12);

    for(int i = 0; i < 12; i++)
    {
        bool res = Dequeue(&queue, (uint8_t*)&dequeue);
        CHECK_FALSE(res);
        LONGS_EQUAL(0xFEEDBEEF, dequeue);
    }

    CheckFrontAndCount(12, 0);
}

/*
 * Test that IsEmpty works
 */
TEST(Queue, IsEmpty)
{
    uint32_t val = 0xDEADBEEF;

    CHECK_TRUE(QueueIsEmpty(&queue));

    Enqueue(&queue, (uint8_t*)&val);

    CHECK_FALSE(QueueIsEmpty(&queue));
}

/*
 * Test that IsFull works
 */
TEST(Queue, IsFull)
{
    uint32_t val = 0xDEADBEEF;

    CHECK_FALSE(QueueIsFull(&queue));

    for(int i = 0; i < 25; i++)
    {
        Enqueue(&queue, (uint8_t*)&val);
    }

    CHECK_TRUE(QueueIsFull(&queue));
}
