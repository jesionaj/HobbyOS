// 2015 Adam Jesionowski

#include "port.h"
#include "rtos.h"
#include "idleTask.h"
#include "task.h"

uintd_t* InitStack(uintd_t* StackPtr, void* func)
{
	// Ready the stack here
	return StackPtr;
}

void PortStartHardwareTimer(TIME time)
{
	// Start the hardware timer to interrupt in time counts
}

// These need to clear their flags

// Have this be called by the timer compare interrupt
void HardwareTimerInterrupt()
{
    TimerInterrupt();
}

// Have this be called by a timer interrupt
void TickTimerInterrupt()
{
    Tick();
}

// Have this be called by a software interrupt
void ReleaseControl()
{
    SwitchToNextAvailableTask();
}
