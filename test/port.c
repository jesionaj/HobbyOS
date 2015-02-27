// 2015 Adam Jesionowski

#include "port.h"
#include "rtos.h"
#include "idleTask.h"
#include "task.h"

volatile uintd_t* InitStack(volatile uintd_t* StackPtr, void* func)
{
	return StackPtr;
}

void ReleaseControl()
{
	SwitchToNextAvailableTask();
}

TIME timerReg;
TIME hwTime;

void PortStartHardwareTimer(TIME time)
{
	hwTime = time;
}
