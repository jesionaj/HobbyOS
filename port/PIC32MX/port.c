// 2015 Adam Jesionowski

#include <xc.h>
#include <plib.h>
#include "port.h"
#include "rtos.h"
#include "idleTask.h"
#include "timer.h"
#include "task.h"

volatile uint32_t* InitStack(volatile uint32_t* StackPtr, void* func)
{
    // Top of stack marker
    *StackPtr = 0xFEEDBEEF;
    StackPtr--;

    // Leave first 20 registers as 0
    StackPtr -= 29;
    // 2: Cause
    // 1: Status
    // 0: EPC

    *StackPtr = _CP0_GET_CAUSE(); // Set to whatever current cause is
    StackPtr--;

    *StackPtr = 0x01; // Enable interrupts
    StackPtr--;

    *StackPtr = (uint32_t)func; // EPC is address of function

    return StackPtr;
}

// Configure RTOS timer interrupt
void InitTickTimer()
{
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_1 | T1_INT_SUB_PRIOR_0);
    OpenTimer1(T1_ON | T1_IDLE_CON | T1_PS_1_8, 2500); // 1 mS
}

// Occurs every second
extern void __ISR(_TIMER_1_VECTOR, ipl1) TickTimerInterruptWrapper(void);
void TickTimerInterrupt()
{
    Tick();

    // Clear the interrupt flag
    INTClearFlag(INT_T1);
}

// Configure core timer interrupt
void InitSoftwareInterrupt()
{
    // Core timer operates at SysClk/2
    OpenCoreTimer(0);

    // Configure Software Interrupt
    INTSetVectorPriority(INT_CORE_SOFTWARE_0_VECTOR, INT_PRIORITY_LEVEL_2);
    INTSetVectorSubPriority(INT_CORE_SOFTWARE_0_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTClearFlag(INT_CS0);
    INTEnable(INT_CS0, INT_ENABLED);
}

// Software interrupt for switching tasks
void __ISR(_CORE_SOFTWARE_0_VECTOR, ipl2) ReleaseControlWrapper(void);
void ReleaseControl()
{
    SwitchToNextAvailableTask();

    // Clear interupt flag
    INTClearFlag(INT_CS0);
}

// Configure hardware timer
void InitHardwareTimer()
{
   // We start off with the interrupt disabled. It will be enabled via PortStartHardwareTimer
    mConfigIntCoreTimer(CT_INT_OFF | CT_INT_PRIOR_2 | CT_INT_SUB_PRIOR_0);
}

// Start up the hardware timer for the passed counts
void PortStartHardwareTimer(TIME time)
{

    // The following code is from http://www.microchip.com/forums/m672888.aspx
    // The library function UpdateCoreTimer doesn't do what we want, which is to
    // add time based on the current count, not the old period.
    unsigned int current_count;

    ENTER_CRITICAL_SECTION;
    // get the current count value 
    asm volatile("mfc0   %0, $9" : "=r"(current_count)); 
    time += current_count;
    // set up the period in the compare reg 
    asm volatile("mtc0   %0,$11" : "+r"(time));
    EXIT_CRITICAL_SECTION;

    mEnableIntCoreTimer();
}

// Timer matched previously set counts, call timer interrupt
void __ISR(_CORE_TIMER_VECTOR, ipl3) CoreTimerInterruptWrapper(void);
void CoreTimerInterrupt()
{
    mDisableIntCoreTimer();
    INTClearFlag(INT_CT);

    TimerInterrupt();
}

