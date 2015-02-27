// 2015 Adam Jesionowski

#include "CppUTest/TestHarness.h"
#include "timer.h"
#include "task.h"
#include "rtos.h"
#include "config.h"
#include <iostream>

extern Timer_t* nextTimer;
extern TIME		timeTimerSet;
extern TIME		hwTime;
extern Timer_t timers[NUM_TIMERS];

Timer_t* timer1;
Timer_t* timer2;
Timer_t* timer3;

bool t1Cb;
bool t2Cb;
bool t3Cb;

void timer1Callback(){ t1Cb = true; }
void timer2Callback(){ t2Cb = true; }
void timer3Callback(){ t3Cb = true; }

TEST_GROUP(Timer)
{

	void setup()
	{
		RTOS_Initialize();

		timer1 = &timers[0];
		timer2 = &timers[1];
		timer3 = &timers[2];

		timeTimerSet = 0;
		timerReg = 0;
		nextTimer = NULL;
		hwTime = 0;

		for(int i = 0; i < NUM_TIMERS; i++)
		{
			timers[i].isActive = false;
			timers[i].timeLeft = 0;
			timers[i].originalTime = 0;
			timers[i].reload = false;
			timers[i].callback = NULL;
		}

		t1Cb = false;
		t2Cb = false;
		t3Cb = false;
	}

	void teardown()
	{

	}
};

TEST(Timer, StartHardwareTimer)
{
	StartHardwareTimer(100);
	LONGS_EQUAL(100, hwTime);
}


TEST(Timer, TimerEnableNoOtherTimer)
{
	TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);

	LONGS_EQUAL(3, timer1->timeLeft);
	CHECK_TRUE(timer1->isActive);
	CHECK_FALSE(t1Cb);

	POINTERS_EQUAL(timer1, nextTimer);
	LONGS_EQUAL(3, hwTime);
}

TEST(Timer, TimerEnableSecondLonger)
{
	TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);
	TimerEnable(SWTimer2, 5, (timerCallback)&timer2Callback, false);

	LONGS_EQUAL(5, timer2->timeLeft);
	CHECK_TRUE(timer2->isActive);
	CHECK_FALSE(t2Cb);

	POINTERS_EQUAL(timer1, nextTimer);
	LONGS_EQUAL(3, hwTime);
}

TEST(Timer, TimerEnableSecondShorter)
{
	TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);
	TimerEnable(SWTimer2, 1, (timerCallback)&timer2Callback, false);

	POINTERS_EQUAL(timer2, nextTimer);
	LONGS_EQUAL(1, hwTime);
}

TEST(Timer, TimerDisableNoOtherTimer)
{
	TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);

	CHECK_TRUE(timer1->isActive);
	POINTERS_EQUAL(timer1, nextTimer);

	TimerDisable(SWTimer1);

	CHECK_FALSE(timer1->isActive);
	POINTERS_EQUAL(NULL, nextTimer);
}

TEST(Timer, TimerDisableSecondLonger)
{
	TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);
	TimerEnable(SWTimer2, 5, (timerCallback)&timer2Callback, false);

	CHECK_TRUE(timer1->isActive);
	POINTERS_EQUAL(timer1, nextTimer);

	TimerDisable(SWTimer1);

	CHECK_FALSE(timer1->isActive);
	POINTERS_EQUAL(timer2, nextTimer);
	LONGS_EQUAL(5, hwTime);
}

TEST(Timer, TimerDisableSecondShorder)
{
	TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);
	TimerEnable(SWTimer2, 1, (timerCallback)&timer2Callback, false);

	CHECK_TRUE(timer1->isActive);
	POINTERS_EQUAL(timer2, nextTimer);

	TimerDisable(SWTimer1);

	CHECK_FALSE(timer1->isActive);
	POINTERS_EQUAL(timer2, nextTimer);
	LONGS_EQUAL(1, hwTime);
}

TEST(Timer, TimerUpdate)
{
	TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);
	TimerEnable(SWTimer2, 4, (timerCallback)&timer2Callback, false);
	TimerEnable(SWTimer3, 7, (timerCallback)&timer3Callback, false);

	timerReg = 5;

	TimerUpdate();

	LONGS_EQUAL(0, timer1->timeLeft);
	CHECK_FALSE(timer1->isActive);
	CHECK_TRUE(t1Cb);

	LONGS_EQUAL(0, timer2->timeLeft);
	CHECK_FALSE(timer2->isActive);
	CHECK_TRUE(t2Cb);

	LONGS_EQUAL(2, timer3->timeLeft);
	CHECK_TRUE(timer3->isActive);
	CHECK_FALSE(t3Cb);

	POINTERS_EQUAL(timer3, nextTimer);
}

TEST(Timer, TimerInterrupt)
{
	TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);
	TimerEnable(SWTimer2, 5, (timerCallback)&timer2Callback, false);
	TimerEnable(SWTimer3, 7, (timerCallback)&timer3Callback, false);

	timerReg = 5;

	TimerInterrupt();

	LONGS_EQUAL(0, timer1->timeLeft);
	CHECK_FALSE(timer1->isActive);
	CHECK_TRUE(t1Cb);

	LONGS_EQUAL(0, timer2->timeLeft);
	CHECK_FALSE(timer2->isActive);
	CHECK_TRUE(t2Cb);

	LONGS_EQUAL(2, timer3->timeLeft);
	CHECK_TRUE(timer3->isActive);
	CHECK_FALSE(t3Cb);

	POINTERS_EQUAL(timer3, nextTimer);

	LONGS_EQUAL(2, hwTime);
	LONGS_EQUAL(5, timeTimerSet);
}

TEST(Timer, TimerReload)
{
	TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, true);

	LONGS_EQUAL(3, timer1->timeLeft);
	CHECK_TRUE(timer1->isActive);
	CHECK_FALSE(t1Cb);

	timerReg = 5;

	TimerInterrupt();

	LONGS_EQUAL(3, timer1->timeLeft);
	CHECK_TRUE(timer1->isActive);
	CHECK_TRUE(t1Cb);

	POINTERS_EQUAL(timer1, nextTimer);
	LONGS_EQUAL(3, hwTime);
}

