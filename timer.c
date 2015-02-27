// 2015 Adam Jesionowski

#include "config.h"
#include "port.h"
#include "timer.h"
#include "rtos.h"

// Non-static due to testing.
Timer_t  timers[NUM_TIMERS];
volatile Timer_t* nextTimer;
volatile TIME     timeTimerSet;

/*
 * This function mainly serves as a wrapper for setting the hardware timer.
 */
void StartHardwareTimer(TIME time)
{
    timeTimerSet = READ_TIMER_REGISTER();
    
    PortStartHardwareTimer(time);
}

/*
 * Starts a timer.
 */
void TimerEnable(SW_TIMER timer, TIME time, timerCallback callback, bool reload)
{
    ENTER_CRITICAL_SECTION;

    Timer_t* t = &timers[timer];

    t->isActive     = true;
    t->timeLeft     = time;
    t->originalTime = time;
    t->callback     = callback;
    t->reload       = reload;

    if(nextTimer == NULL)
    {
    	// If there is no next timer, make it this one
        nextTimer = t;
        StartHardwareTimer(time);
    }
    else if(time < nextTimer->timeLeft)
    {
    	// Set this timer as next if it has less time remaining. TimerUpdate will set this
        TimerUpdate();
        StartHardwareTimer(time);
    }

    EXIT_CRITICAL_SECTION;
}

/*
 * Starts a timer while inside a timer callback
 */
void TimerEnableInCallback(SW_TIMER timer, TIME time, timerCallback callback, bool reload)
{
    Timer_t* t = &timers[timer];

    t->isActive     = true;
    t->timeLeft     = time;
    t->originalTime = time;
    t->callback     = callback;
    t->reload       = reload;

    // Set nextTimer if applicable
    if(time < nextTimer->timeLeft)
    {
    	nextTimer = t;
    }
}

/*
 * Stops a timer.
 */
void TimerDisable(SW_TIMER timer)
{
    ENTER_CRITICAL_SECTION;

    Timer_t* t = &timers[timer];

    if(t->isActive)
    {
        t->isActive = false;

        // If this was supposed to be the next timer to trigger, handle that
        if(t == nextTimer)
        {
            TimerUpdate();

            // If there's a next timer, start hw timer again
            if(nextTimer)
            {
                StartHardwareTimer(nextTimer->timeLeft);
            }
        }
    }

    EXIT_CRITICAL_SECTION;
}

/*
 * Timers themselves do not tick down at regular intervals. Instead, when anything occurs
 * that requires the timer values or nextTimer to change (hardware timer interrupt occurs,
 * a new timer is added), then this function is called to update the time left on each timer
 * and set the nextTimer that needs to fire.
 */
void TimerUpdate()
{
    uintd_t i;
    TIME differential;
    TIME timeNow = READ_TIMER_REGISTER();

    // When we update, use a fake timer as the next one
    static Timer_t lastTimer = {
        false,
        false,
        TIMER_MAX,
        TIMER_MAX,
        NULL
    };

    nextTimer = &lastTimer;

    if(timeNow < timeTimerSet)
    {
        // In this case, the timer has overflowed since it was last set. Calculate the differential accordingly.
        differential = (TIMER_MAX - timeTimerSet) + timeNow;
    }
    else
    {
        differential = timeNow - timeTimerSet;
    }

    for(i = 0; i < NUM_TIMERS; i++)
    {
        Timer_t* t = &timers[i];

        // We only care about active timers
        if(t->isActive)
        {
            // If differential is less than the time left, just subtract it and move on
            if(differential < t->timeLeft)
            {
                t->timeLeft -= differential;

                // Check if this should be nextTimer
                if(t->timeLeft < nextTimer->timeLeft)
                {
                    nextTimer = t;
                }
            }
            else
            {
            	// If this timer auto-reloads, do so
                if(t->reload)
                {
                    t->timeLeft = t->originalTime;

                    // Also check if this one should be nextTimer
                    if(t->timeLeft < nextTimer->timeLeft)
                    {
                        nextTimer = t;
                    }
                }
                else
                {
                    // Otherwise, this timer is done.
                    t->isActive = false;
                    t->timeLeft = 0;
                }

                // Call the callback function
            	t->callback();
            }
        }
    }

    // If nextTimer did not change, set it to NULL
    if(nextTimer == &lastTimer)
    {
        nextTimer = NULL;
    }
}

/*
 * When a hardware interrupt occurs, this function will be called. It updates the timers and then
 * restarts the hardware timer if necessary.
 */
void TimerInterrupt()
{
    // There should always be a next timer, but check anyway
    if(nextTimer)
    {
    	// This will handle notifying the completed timer
        TimerUpdate();

        // And start again if there's a next timer
        if(nextTimer)
        {
            StartHardwareTimer(nextTimer->timeLeft);
        }
    }
}
