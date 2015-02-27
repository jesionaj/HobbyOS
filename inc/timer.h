// 2015 Adam Jesionowski

/*
 * DelayCurrentTasks allows tasks to wait for a certain number of milliseconds, aligned with the
 * Tick function. Software timers allow for much finer grained timing control (based on the hardware
 * implementation). In the PIC32 implementation, this means events can trigger with ~50 ns precision
 * (truthfully much less, as RTOS overhead is ~8 us).
 *
 * When the timer fires, it will call the passed timerCallback function. If reload is True, it will
 * automatically reload the timer, otherwise the timer will then be disabled.
 *
 * Setting times are hardware specific, as they represent hardware timer counts.
 *
 * When writing callback functions, it's important to remember that they occur on the OS stack,
 * not associated with any task.
 *
 * To create a new timer, add it to the SW_TIMER enum in config.h.
 *
 * Based on Implementing Software Timers, Don Libes
 * http://www.kohala.com/start/libes.timers.txt
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "config.h"
#include "list.h"

#ifdef	__cplusplus
extern "C" {
#endif

// Timer callbacks take no values and return nothing.
typedef void (*timerCallback)(void);

typedef struct _timer_t
{
	bool            isActive;       // Whether the timer being used or not
	bool            reload;         // If true, this timer will continually fire
	TIME            timeLeft;       // Time left on the timer
	TIME            originalTime;   // Value passed when TimerEnable is called, used if reload is true
	timerCallback	callback;       // Function called when timeLeft reaches zero
} Timer_t;

void TimerEnable(SW_TIMER timer, TIME time, timerCallback callback, bool reload);
void TimerDisable(SW_TIMER timer);
void TimerInterrupt();

// These are exposed for testing purposes.
void TimerUpdate();
void StartHardwareTimer(TIME time);

#ifdef	__cplusplus
}
#endif

#endif /* TIMER_H_ */
