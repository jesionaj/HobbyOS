// 2015 Adam Jesionowski

/*
 * The idle task just waits in a while(1) loop at the lowest possible priority.
 */

#ifndef IDLETASK_H_
#define IDLETASK_H_

#include "task.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern Task_t idleTask;

void IdleTask_init();
void IdleTask_main();

#ifdef	__cplusplus
}
#endif


#endif /* IDLETASK_H_ */
