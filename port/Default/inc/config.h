// 2015 Adam Jesionowski

#ifndef CONFIG_H
#define	CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef	NULL
    #define NULL (0)
#endif	/* NULL */

// Default size type (should be equal to width of processor)
typedef uint32_t uintd_t;

// Priority
#define NUM_PRIORITY_LEVELS 7
#define PRIORITY_0 0
#define PRIORITY_1 1
#define PRIORITY_2 2
#define PRIORITY_3 3
#define PRIORITY_4 4
#define PRIORITY_5 5
#define PRIORITY_6 6

#define PRIORITY_IDLE PRIORITY_0

// Stack
#define DFLT_STACK_SIZE	200
#define OS_STACK_SIZE	800

// Define this if you're going to run unit tests.
#define RUNTESTS


typedef enum {
	SWTimer1 = 0,
	SWTimer2,
	SWTimer3,
	NUM_TIMERS
} SW_TIMER;

typedef uintd_t TIME;
#define TIMER_MAX  4294967295U // 32-bit uint max

#ifdef RUNTESTS
    #define LOOP(b)
#else
    #define LOOP(b) while(b)
#endif



#ifdef	__cplusplus
}
#endif

#endif	/* PORT_TYPES_H */

