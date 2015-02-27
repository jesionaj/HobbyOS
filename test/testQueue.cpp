// 2015 Adam Jesionowski

#include "CppUTest/TestHarness.h"
#include "queue.h"
#include <iostream>

#define SIZE 25

TEST_GROUP(Queue)
{

	Queue_t queue;
	uint32_t data[25];

	void setup()
	{

		InitQueue(&queue, (uint8_t*)data, sizeof(uint32_t), SIZE);
	}
	void teardown()
	{

	}


};

TEST(Queue, InitQueue)
{
	// 25 uint32_ts


	POINTERS_EQUAL(data, queue.start);
	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(0, queue.count);
	LONGS_EQUAL(25, queue.maxSize);
	LONGS_EQUAL(sizeof(uint32_t), queue.sizeOf);
	POINTERS_EQUAL(NULL, queue.tasksBlockedOnRead);
	POINTERS_EQUAL(NULL, queue.tasksBlockedOnWrite);
}

TEST(Queue, EnqueueOne)
{
	uint32_t val = 0xDEADBEEF;
	bool res = Enqueue(&queue, (uint8_t*)&val);

	CHECK_FALSE(res);
	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(1, queue.count);
	LONGS_EQUAL(0xDEADBEEF, *(uint32_t*)data);
}

TEST(Queue, EnqueueFull)
{
	uint32_t val = 0xDEADBEEF;

	for(int i = 0; i < 25; i++)
	{

		bool res = Enqueue(&queue, (uint8_t*)&val);
		CHECK_FALSE(res);
	}

	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(25, queue.count);
	for(int i = 0; i < 25; i++)
	{
		LONGS_EQUAL(0xDEADBEEF, (data[i]));
	}
}

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

	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(25, queue.count);
	for(int i = 0; i < 25; i++)
	{
		LONGS_EQUAL(0xDEADBEEF, data[i]);
	}
}

TEST(Queue, DequeueEmpty)
{
	uint32_t val = 0;
	bool res = Dequeue(&queue, (uint8_t*)&val);

	CHECK_TRUE(res);
	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(0, queue.count);
}

TEST(Queue, DequeueOne)
{
	uint32_t insert = 0xDEADBEEF;
	uint32_t dequeue = 0;

	bool res = Enqueue(&queue, (uint8_t*)&insert);
	CHECK_FALSE(res);

	res = Dequeue(&queue, (uint8_t*)&dequeue);
	CHECK_FALSE(res);

	LONGS_EQUAL(1, queue.front);
	LONGS_EQUAL(0, queue.count);
	LONGS_EQUAL(0xDEADBEEF, dequeue);
}

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

	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(4, queue.count);


	for(int i = 0; i < 2; i++)
	{
		res = Dequeue(&queue, (uint8_t*)&dequeue);
		CHECK_FALSE(res);
		LONGS_EQUAL(100+i, dequeue);
	}

	LONGS_EQUAL(2, queue.front);
	LONGS_EQUAL(2, queue.count);

	for(int i = 0; i < 2; i++)
	{
		res = Enqueue(&queue, (uint8_t*)&insert);
		CHECK_FALSE(res);
		insert++;
	}

	LONGS_EQUAL(2, queue.front);
	LONGS_EQUAL(4, queue.count);

	for(int i = 0; i < 4; i++)
	{
		res = Dequeue(&queue, (uint8_t*)&dequeue);
		CHECK_FALSE(res);
		LONGS_EQUAL(102+i, dequeue);
	}

	LONGS_EQUAL(6, queue.front);
	LONGS_EQUAL(0, queue.count);
}

TEST(Queue, CircularTest)
{
	uint32_t insert = 0xDEADBEEF;
	uint32_t dequeue = 0;

	for(int i = 0; i < 25; i++)
	{
		bool res = Enqueue(&queue, (uint8_t*)&insert);
		CHECK_FALSE(res);
	}

	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(25, queue.count);

	for(int i = 0; i < 12; i++)
	{
		bool res = Dequeue(&queue, (uint8_t*)&dequeue);
		CHECK_FALSE(res);
		LONGS_EQUAL(0xDEADBEEF, dequeue);
	}

	LONGS_EQUAL(12, queue.front);
	LONGS_EQUAL(13, queue.count);

	insert = 0xFEEDBEEF;
	for(int i = 0; i < 12; i++)
	{
		bool res = Enqueue(&queue, (uint8_t*)&insert);
		CHECK_FALSE(res);
	}

	LONGS_EQUAL(12, queue.front);
	LONGS_EQUAL(25, queue.count);

	for(int i = 0; i < 13; i++)
	{
		bool res = Dequeue(&queue, (uint8_t*)&dequeue);
		CHECK_FALSE(res);
		LONGS_EQUAL(0xDEADBEEF, dequeue);
	}

	LONGS_EQUAL(0, queue.front);
	LONGS_EQUAL(12, queue.count);

	for(int i = 0; i < 12; i++)
	{
		bool res = Dequeue(&queue, (uint8_t*)&dequeue);
		CHECK_FALSE(res);
		LONGS_EQUAL(0xFEEDBEEF, dequeue);
	}

	LONGS_EQUAL(12, queue.front);
	LONGS_EQUAL(0, queue.count);

}

TEST(Queue, IsEmpty)
{
	uint32_t val = 0xDEADBEEF;

	CHECK_TRUE(QueueIsEmpty(&queue));

	Enqueue(&queue, (uint8_t*)&val);

	CHECK_FALSE(QueueIsEmpty(&queue));
}

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
