// 2015 Adam Jesionowski

#include "idleTask.h"
#include "config.h"
#include "port.h"

static uintd_t IdleTask_stack[DFLT_STACK_SIZE];

Task_t idleTask =
{
    PRIORITY_IDLE,
    {NULL, NULL, &idleTask},
    0,
    IdleTask_stack
};

void IdleTask_init()
{
    idleTask.stackPtr = &IdleTask_stack[DFLT_STACK_SIZE-1];
    idleTask.stackPtr = (uintd_t*)InitStack(idleTask.stackPtr, IdleTask_main);
}

void IdleTask_main()
{
    // Potentially this could be replaced with 'hlt' or some other portable power down thing
    // For now, just leave it as this
    while(1);
}
