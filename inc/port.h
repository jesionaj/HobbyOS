// 2015 Adam Jesionowski

#ifndef PORT_H_
#define PORT_H_

#include "config.h"

void InitHardwareTimer();
void InitTickTimer();
void InitSoftwareInterrupt();

volatile uintd_t* InitStack(volatile uintd_t* StackPtr, void* func);

void PortStartHardwareTimer(TIME time);

#endif /* PORT_H_ */
