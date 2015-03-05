// 2015 Adam Jesionowski

#include "CppUTest/TestHarness.h"
#include "timer.h"
#include "task.h"
#include "rtos.h"
#include "config.h"
#include <iostream>

// Fake hardware
extern TIME hwTime;     // Fake hardware register compare
extern TIME timerReg;   // Fake hardware register count

// Variables from timer.c
extern TIME timeTimerSet;
extern Timer_t* nextTimer;
extern Timer_t timers[NUM_TIMERS];

// Convenience pointers to the first 3 timers.
Timer_t* timer1 = NULL;
Timer_t* timer2 = NULL;
Timer_t* timer3 = NULL;

// These booleans indicate if a callback has succeeded.
bool t1Cb = false;
bool t2Cb = false;
bool t3Cb = false;

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

    void SetTimerCount(TIME count)
    {
        timerReg = count;
    }

    void CheckTimerCompare(TIME expected)
    {
        LONGS_EQUAL(expected, hwTime);
    }

    void CheckTimeSet(TIME expected)
    {
        LONGS_EQUAL(expected, timeTimerSet);
    }

    void CheckNextTimer(Timer_t* expected)
    {
        POINTERS_EQUAL(expected, nextTimer);
    }
};

/*
 * Check that calling start hardware timer sets the compare register
 */
TEST(Timer, StartHardwareTimer)
{
    StartHardwareTimer(100);
    CheckTimerCompare(100);
}


/*
 * Enable a single timer.
 */
TEST(Timer, TimerEnableNoOtherTimer)
{
    TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);

    // Timer1 will be next timer, with 3 left on itself and the hardware compare.
    LONGS_EQUAL(3, timer1->timeLeft);
    CHECK_TRUE(timer1->isActive);
    CHECK_FALSE(t1Cb);

    CheckNextTimer(timer1);
    CheckTimerCompare(3);
}

/*
 * Enable two timers, the second one with a longer period.
 */
TEST(Timer, TimerEnableSecondLonger)
{
    TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);
    TimerEnable(SWTimer2, 5, (timerCallback)&timer2Callback, false);

    // Timer1 will still be next timer, with 3 left on hardware compare.
    LONGS_EQUAL(5, timer2->timeLeft);
    CHECK_TRUE(timer2->isActive);
    CHECK_FALSE(t2Cb);

    CheckNextTimer(timer1);
    CheckTimerCompare(3);
}

/*
 * Enable two timers, the second one with a shorter period.
 */
TEST(Timer, TimerEnableSecondShorter)
{
    TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);
    TimerEnable(SWTimer2, 1, (timerCallback)&timer2Callback, false);

    // Now Timer2 will be next timer, with 1 left on hardware compare.
    CheckNextTimer(timer2);
    CheckTimerCompare(1);
}

/*
 * Enable, then disable a single timer.
 */
TEST(Timer, TimerDisableNoOtherTimer)
{
    TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);

    CHECK_TRUE(timer1->isActive);
    CheckNextTimer(timer1);

    TimerDisable(SWTimer1);

    // There should be no next timer now.
    CHECK_FALSE(timer1->isActive);
    CheckNextTimer(NULL);
}

/*
 * Enable two timers, then disable the one with a shorter period.
 */
TEST(Timer, TimerDisableShorter)
{
    TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);
    TimerEnable(SWTimer2, 5, (timerCallback)&timer2Callback, false);

    CHECK_TRUE(timer1->isActive);
    CheckNextTimer(timer1);

    TimerDisable(SWTimer1);

    // Now timer1 should be inactive, and timer2 should be next, with 5 left.
    CHECK_FALSE(timer1->isActive);
    CheckNextTimer(timer2);
    CheckTimerCompare(5);
}

/*
 * Enable two timers, then disable the one with the longer period.
 */
TEST(Timer, TimerDisableLonger)
{
    TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);
    TimerEnable(SWTimer2, 1, (timerCallback)&timer2Callback, false);

    CHECK_TRUE(timer1->isActive);
    CheckNextTimer(timer2);

    TimerDisable(SWTimer1);

    // Again, timer1 should be inactive, and timer2 should be next, with 1 left.
    CHECK_FALSE(timer1->isActive);
    CheckNextTimer(timer2);
    CheckTimerCompare(1);
}

/*
 * Test timer update call.
 */
TEST(Timer, TimerUpdate)
{
    TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);
    TimerEnable(SWTimer2, 4, (timerCallback)&timer2Callback, false);
    TimerEnable(SWTimer3, 7, (timerCallback)&timer3Callback, false);

    // We set register to 5, so timers 1 and 2 should fire, but timer 3 should not.
    SetTimerCount(5);

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

    CheckNextTimer(timer3);
}

/*
 * Timer interrupt is almost identical to timer update, but also updates the hardware timers
 */
TEST(Timer, TimerInterrupt)
{
    TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, false);
    TimerEnable(SWTimer2, 5, (timerCallback)&timer2Callback, false);
    TimerEnable(SWTimer3, 7, (timerCallback)&timer3Callback, false);

    SetTimerCount(5);

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

    CheckNextTimer(timer3);

    CheckTimerCompare(2);
    CheckTimeSet(5);
}

/*
 * Test auto-reloading timers
 */
TEST(Timer, TimerReload)
{
    TimerEnable(SWTimer1, 3, (timerCallback)&timer1Callback, true);

    LONGS_EQUAL(3, timer1->timeLeft);
    CHECK_TRUE(timer1->isActive);
    CHECK_FALSE(t1Cb);

    // Fire timer
    SetTimerCount(5);

    TimerInterrupt();

    // The timer should still be active. Note that 3 is still time left, not 2. This is on purpose, we always reload with the
    // original value, regardless of how much time has passed.
    LONGS_EQUAL(3, timer1->timeLeft);
    CHECK_TRUE(timer1->isActive);
    CHECK_TRUE(t1Cb);

    CheckNextTimer(timer1);
    CheckTimerCompare(3);
}

