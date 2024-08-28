/*
 * task.h
 *
 *  Created on: Aug 25, 2024
 *      Author: Abdelrahman
 */

#ifndef INCLUDE_TASK_H_
#define INCLUDE_TASK_H_


typedef void (*TaskFunction_t)(void* parameters);
typedef unsigned char mPriority_t;
typedef unsigned int stackSize_t;
typedef char tskName_t;

typedef enum {
	Suspended,
	Waiting,
	Ready,
	Running
}tskStatus_t;


typedef struct {
	unsigned int tskStackSize;
	mPriority_t tskPriority;
	unsigned int tskStackStart;
	unsigned int tskStackEnd;
	char tskName[30];
	unsigned int *tskCurrentPSP;
	TaskFunction_t taskFunc;

	struct {
		enum {
			Enable,
			Disable,
		}Blocking;
		unsigned int ticks_count;
	}TimeWaiting;

	tskStatus_t tskStatus;
}mTCB_t;

typedef mTCB_t* mTaskHandle_t;




#endif /* INCLUDE_TASK_H_ */
