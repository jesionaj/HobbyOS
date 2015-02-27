// 2015 Adam Jesionowski

// Utility class for testing

#ifndef UTILS_H_
#define UTILS_H_

#include "list.h"
#include "task.h"

List_t* makeNode(void* owner);
Task_t* makeTask(uintd_t timer, uint8_t prio);

#endif /* UTILS_H_ */
