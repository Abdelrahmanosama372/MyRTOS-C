/*
 * scheduler.h
 *
 *  Created on: Aug 25, 2024
 *      Author: Abdelrahman
 */

#ifndef INCLUDE_SCHEDULER_H_
#define INCLUDE_SCHEDULER_H_

extern int _estack;
extern int _ebss;

#include "task.h"
#include "CortexMX_Porting.h"
#include "MYRTOS_FIFO.h"

#define OS_STACK_SIZE 3072 // 3KB
#define READY_QUEUE_SIZE 10
#define MAX_TASKS_NUMBER 10

typedef enum {
	TaskCreationError,
	TaskCreationSuccess,
}MYRTOS_Status;

typedef unsigned char bool;

#define true  1
#define false 0

//--------------------------------------------
//------------------- APIs -------------------
//--------------------------------------------
void MYRTOS_Init();
void MYRTOS_Start();
void MYRTOS_activateTask(mTaskHandle_t taskHandle);
void MYRTOS_suspendTask(mTaskHandle_t taskHandle);
void MYRTOS_terminateTask(mTaskHandle_t taskHandle);
void MYRTOS_delayTask(mTaskHandle_t taskHandle, unsigned int ticks_count);
mTaskHandle_t getCurrentTask();
//const mTaskHandle_t getCurrentTask();

MYRTOS_Status MYRTOS_createTask(TaskFunction_t task,
						mPriority_t tskPriority,
						stackSize_t stackSize,
						tskName_t *name,
						void **tskParameters,
						mTaskHandle_t *handle);

typedef enum {
	ActivateTask,
	TerminateTask,
	SuspendTask,
	TaskWaitingTime,
}SVC_ID_t;

void OS_SVC_Set(SVC_ID_t id);
void OS_SVC(int *stackFrame);


#endif /* INCLUDE_SCHEDULER_H_ */
