// 2015 Adam Jesionowski

#include "CppUTest/TestHarness.h"
#include "queue.h"
#include "rtos.h"
#include "idleTask.h"
#include <iostream>

TEST_GROUP(BlockingQueue)
{

	Queue_t queue;
	uint8_t data[100];

	void setup()
	{
		RTOS_Initialize();
		StartTask(&idleTask);
		Tick();
		InitQueue(&queue, data, sizeof(uint32_t), 25);
	}
	void teardown()
	{

	}
};

TEST(BlockingQueue, BlockOnEmpty)
{
	uint32_t val = 0;

	DequeueBlocking(&queue, (uint8_t*)&val);

	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(0, queue.count);

	POINTERS_EQUAL(queue.tasksBlockedOnRead, &idleTask.taskList);
}

TEST(BlockingQueue, BlockOnFull)
{
	uint32_t val = 0xDEADBEEF;

	for(int i = 0; i < 25; i++)
	{
		EnqueueBlocking(&queue, (uint8_t*)&val);
	}

	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(25, queue.count);

	EnqueueBlocking(&queue, (uint8_t*)&val);

	POINTERS_EQUAL(queue.tasksBlockedOnWrite, &idleTask.taskList);
}


// Below are tests from testQueue replicated with the blocking methods that don't run into a block
TEST(BlockingQueue, EnqueueOne)
{
	uint32_t val = 0xDEADBEEF;
	EnqueueBlocking(&queue, (uint8_t*)&val);

	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(1, queue.count);
	LONGS_EQUAL(0xDEADBEEF, *(uint32_t*)data);
}

TEST(BlockingQueue, EnqueueFull)
{
	uint32_t val = 0xDEADBEEF;

	for(int i = 0; i < 25; i++)
	{
		EnqueueBlocking(&queue, (uint8_t*)&val);
	}

	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(25, queue.count);
	for(int i = 0; i < 25; i++)
	{
		LONGS_EQUAL(0xDEADBEEF, *(uint32_t*)(data+i*sizeof(uint32_t)));
	}
}

TEST(BlockingQueue, DequeueOne)
{
	uint32_t insert = 0xDEADBEEF;
	uint32_t dequeue = 0;

	EnqueueBlocking(&queue, (uint8_t*)&insert);
	DequeueBlocking(&queue, (uint8_t*)&dequeue);

	LONGS_EQUAL(1, queue.front);
	LONGS_EQUAL(0, queue.count);
	LONGS_EQUAL(0xDEADBEEF, dequeue);
}

TEST(BlockingQueue, DequeueEnqueueMix)
{
	// Enqueue 4, Dequeue 2, Enqueue 2, Dequeue 6

	uint32_t insert = 100;
	uint32_t dequeue = 0;

	for(int i = 0; i < 4; i++)
	{
		EnqueueBlocking(&queue, (uint8_t*)&insert);
		insert++;
	}

	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(4, queue.count);

	for(int i = 0; i < 2; i++)
	{
		DequeueBlocking(&queue, (uint8_t*)&dequeue);
		LONGS_EQUAL(100+i, dequeue);
	}

	LONGS_EQUAL(2, queue.front);
	LONGS_EQUAL(2, queue.count);

	for(int i = 0; i < 2; i++)
	{
		EnqueueBlocking(&queue, (uint8_t*)&insert);
		insert++;
	}

	LONGS_EQUAL(2, queue.front);
	LONGS_EQUAL(4, queue.count);

	for(int i = 0; i < 4; i++)
	{
		DequeueBlocking(&queue, (uint8_t*)&dequeue);
		LONGS_EQUAL(102+i, dequeue);
	}

	LONGS_EQUAL(6, queue.front);
	LONGS_EQUAL(0, queue.count);
}

TEST(BlockingQueue, CircularTest)
{
	uint32_t insert = 0xDEADBEEF;
	uint32_t dequeue = 0;

	for(int i = 0; i < 25; i++)
	{
		EnqueueBlocking(&queue, (uint8_t*)&insert);
	}

	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(25, queue.count);

	for(int i = 0; i < 12; i++)
	{
		DequeueBlocking(&queue, (uint8_t*)&dequeue);
		LONGS_EQUAL(0xDEADBEEF, dequeue);
	}

	LONGS_EQUAL(12, queue.front);
	LONGS_EQUAL(13, queue.count);

	insert = 0xFEEDBEEF;
	for(int i = 0; i < 12; i++)
	{
		EnqueueBlocking(&queue, (uint8_t*)&insert);
	}

	LONGS_EQUAL(12, queue.front);
	LONGS_EQUAL(25, queue.count);

	for(int i = 0; i < 13; i++)
	{
		DequeueBlocking(&queue, (uint8_t*)&dequeue);
		LONGS_EQUAL(0xDEADBEEF, dequeue);
	}

	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(12, queue.count);

	for(int i = 0; i < 12; i++)
	{
		DequeueBlocking(&queue, (uint8_t*)&dequeue);
		LONGS_EQUAL(0xFEEDBEEF, dequeue);
	}

	LONGS_EQUAL(12, queue.front);
	LONGS_EQUAL(0, queue.count);

}
