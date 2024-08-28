/*
 * scheduler.c
 *
 *  Created on: Aug 25, 2024
 *      Author: Abdelrahman
 */

#include "scheduler.h"

//--------------------------------------------
//------------- Global Variables -------------
//--------------------------------------------

mTaskHandle_t tasksHandle[READY_QUEUE_SIZE];
FIFO_Buf_t readyQeue;
mTCB_t tasksCB[MAX_TASKS_NUMBER];
unsigned int noOfTasks = 0;

struct {
	mTaskHandle_t tasksHandle[MAX_TASKS_NUMBER];
	unsigned int stackTop;
	unsigned int stackEnd;
	unsigned int stackLocator;
	mTaskHandle_t currentTask;
	mTaskHandle_t nextTask;

	enum {
		OS_Running,
		OS_Stopped,
	}OS_mode;
}OS_Control;

//--------------------------------------------
//------------- Helper Functions -------------
//--------------------------------------------

bool hasAvailableStackSize(stackSize_t stackSize)
{
	if(OS_Control.stackLocator - stackSize * 4 > (unsigned int)&_ebss)
	{
		return true;
	}else {
		return false;
	}
}

void allocateTaskStack(mTCB_t* taskCB)
{
	taskCB->tskStackStart = OS_Control.stackLocator;

	OS_Control.stackLocator -= taskCB->tskStackSize * 4;

	taskCB->tskStackEnd = OS_Control.stackLocator;

	taskCB->tskCurrentPSP = (unsigned int *)taskCB->tskStackStart;

	OS_Control.stackLocator -= 8;
}

void setTaskStack(mTCB_t* taskCB)
{
	taskCB->tskCurrentPSP--;
	*(taskCB->tskCurrentPSP) = 0x01000000;

	taskCB->tskCurrentPSP--;
	(*taskCB->tskCurrentPSP) = (unsigned int)taskCB->taskFunc;

	taskCB->tskCurrentPSP--;
	(*taskCB->tskCurrentPSP) = 0xFFFFFFED;

	int i;
	for(i=0; i<13; i++)
	{
		taskCB->tskCurrentPSP--;
		(*taskCB->tskCurrentPSP) = 0;
	}
}


//--------------------------------------------
//----------- APIs Implementation ------------
//--------------------------------------------

MYRTOS_Status MYRTOS_createTask(TaskFunction_t task,
						mPriority_t tskPriority,
						stackSize_t stackSize,
						tskName_t *name,
						void **tskParameters,
						mTaskHandle_t *handle)
{
	if(hasAvailableStackSize(stackSize) == false){
		return TaskCreationError;
	}

    mTCB_t* taskCB = &tasksCB[noOfTasks];
	*handle = taskCB;

	OS_Control.tasksHandle[noOfTasks] = taskCB;
	noOfTasks++;

	taskCB->taskFunc = task;
	taskCB->tskCurrentPSP = (unsigned int*)taskCB->tskStackStart;
	taskCB->tskStackSize = stackSize;
	taskCB->tskPriority = tskPriority;
	taskCB->tskStatus = Suspended;
	taskCB->TimeWaiting.Blocking = Disable;
	taskCB->TimeWaiting.ticks_count = 0;

	strcpy(taskCB->tskName, name);

	allocateTaskStack(taskCB);

	setTaskStack(taskCB);

	return TaskCreationSuccess;
}

void OS_createMainStack(){
	OS_Control.stackTop = (unsigned int)&_estack;

	OS_Control.stackEnd = OS_Control.stackTop - OS_STACK_SIZE;

	OS_Control.stackLocator = OS_Control.stackEnd - 8;
}

unsigned int idle_task = 0;

void idleTask(void *parameters)
{

	while(1)
	{
		if (idle_task == 0 ){
			idle_task = 1;
		}else {
			idle_task = 0;
		}
	}
}

void MYRTOS_Init()
{
	OS_createMainStack();

	OS_Control.OS_mode = OS_Stopped;

	FIFO_init(&readyQeue, tasksHandle, READY_QUEUE_SIZE);
}

void MYRTOS_Start()
{
	mTaskHandle_t handle;
	MYRTOS_createTask(idleTask, 255, 20, "idle task", NULL, &handle);

	handle->tskStatus = Ready;

	OS_Control.OS_mode = OS_Running;
	OS_Control.currentTask = handle;
	OS_Control.nextTask = NULL;

	OS_SET_PSP(OS_Control.currentTask->tskStackStart);

	HW_init();

	OS_SWITCH_SP_TO_PSP;
}


void OS_SVC_Set(SVC_ID_t id)
{
	switch(id) {
		case ActivateTask:
			__asm("svc #0x00");
			break;
		case TerminateTask:
			__asm("svc #0x01");
			break;
		case SuspendTask:
			__asm("svc #0x02");
			break;
		case TaskWaitingTime:
			__asm("svc #0x03");
			break;
	}
}


__attribute ((naked)) void PendSV_Handler(void)
{
	// save current task registers values

	OS_GET_PSP(OS_Control.currentTask->tskCurrentPSP);

	OS_Control.currentTask->tskCurrentPSP--;
	__asm volatile ("MOV %0, R4":"=r"(*OS_Control.currentTask->tskCurrentPSP));

	OS_Control.currentTask->tskCurrentPSP--;
	__asm volatile ("MOV %0, R5":"=r"(*OS_Control.currentTask->tskCurrentPSP));

	OS_Control.currentTask->tskCurrentPSP--;
	__asm volatile ("MOV %0, R6":"=r"(*OS_Control.currentTask->tskCurrentPSP));

	OS_Control.currentTask->tskCurrentPSP--;
	__asm volatile ("MOV %0, R7":"=r"(*OS_Control.currentTask->tskCurrentPSP));

	OS_Control.currentTask->tskCurrentPSP--;
	__asm volatile ("MOV %0, R8":"=r"(*OS_Control.currentTask->tskCurrentPSP));

	OS_Control.currentTask->tskCurrentPSP--;
	__asm volatile ("MOV %0, R9":"=r"(*OS_Control.currentTask->tskCurrentPSP));

	OS_Control.currentTask->tskCurrentPSP--;
	__asm volatile ("MOV %0, R10":"=r"(*OS_Control.currentTask->tskCurrentPSP));

	OS_Control.currentTask->tskCurrentPSP--;
	__asm volatile ("MOV %0, R11":"=r"(*OS_Control.currentTask->tskCurrentPSP));

	// restore next task registers values

	if (OS_Control.nextTask != NULL){
		OS_Control.currentTask = OS_Control.nextTask;
//		OS_Control.nextTask = NULL ;
	}

	__asm volatile ("MOV R11, %0": : "r"(*OS_Control.currentTask->tskCurrentPSP));
	OS_Control.currentTask->tskCurrentPSP++;

	__asm volatile ("MOV R10, %0": : "r"(*OS_Control.currentTask->tskCurrentPSP));
	OS_Control.currentTask->tskCurrentPSP++;

	__asm volatile ("MOV R9, %0": : "r"(*OS_Control.currentTask->tskCurrentPSP));
	OS_Control.currentTask->tskCurrentPSP++;

	__asm volatile ("MOV R8, %0": : "r"(*OS_Control.currentTask->tskCurrentPSP));
	OS_Control.currentTask->tskCurrentPSP++;

	__asm volatile ("MOV R7, %0": : "r"(*OS_Control.currentTask->tskCurrentPSP));
	OS_Control.currentTask->tskCurrentPSP++;

	__asm volatile ("MOV R6, %0": : "r"(*OS_Control.currentTask->tskCurrentPSP));
	OS_Control.currentTask->tskCurrentPSP++;

	__asm volatile ("MOV R5, %0": : "r"(*OS_Control.currentTask->tskCurrentPSP));
	OS_Control.currentTask->tskCurrentPSP++;

	__asm volatile ("MOV R4, %0": : "r"(*OS_Control.currentTask->tskCurrentPSP));
	OS_Control.currentTask->tskCurrentPSP++;

	OS_SET_PSP(OS_Control.currentTask->tskCurrentPSP);
	__asm volatile ("BX LR");
}


void OS_tasksBubbleSort()
{
	int i,j,n;
	n = noOfTasks;
	for(i=0; i<n; i++){
		for(j=i; j<n; j++){
			if(OS_Control.tasksHandle[i]->tskPriority > OS_Control.tasksHandle[j]->tskPriority)
			{
				mTaskHandle_t tempHandle = OS_Control.tasksHandle[i];
				OS_Control.tasksHandle[i] = OS_Control.tasksHandle[j];
				OS_Control.tasksHandle[j] = tempHandle;
			}
		}
	}
}


void updateSchedulerTable()
{
	FIFO_dequeueAll(&readyQeue);

	OS_tasksBubbleSort();

	mPriority_t highestPriority = OS_Control.tasksHandle[0]->tskPriority;

	int i = 0;
	while(i < noOfTasks){
		if(OS_Control.tasksHandle[i]->tskStatus != Suspended){
			if(OS_Control.tasksHandle[i]->tskPriority == highestPriority)
			{
					FIFO_enqueue(&readyQeue, OS_Control.tasksHandle[i]);
			}
			else if(i !=0 && OS_Control.tasksHandle[i-1]->tskStatus == Suspended && OS_Control.tasksHandle[i]->tskPriority != 255)
			{
				highestPriority = OS_Control.tasksHandle[i]->tskPriority;
				FIFO_enqueue(&readyQeue, OS_Control.tasksHandle[i]);
			}
			else if(OS_Control.tasksHandle[i]->tskPriority > highestPriority)
			{
				break;
			}
		}
		i++;
	}
}


void OS_decideWhatNext()
{
	if(FIFO_is_empty(&readyQeue) == FIFO_EMPTY && OS_Control.currentTask->tskStatus != Suspended)
	{
		OS_Control.nextTask = OS_Control.currentTask;
	}else {
		 FIFO_dequeue(&readyQeue, &OS_Control.nextTask);
		 OS_Control.nextTask->tskStatus = Running;
		 if(OS_Control.currentTask->tskPriority == OS_Control.nextTask->tskPriority
				 && OS_Control.currentTask->tskStatus != Suspended && OS_Control.currentTask != OS_Control.nextTask)
		 {
			 OS_Control.currentTask->tskStatus = Ready;
			 FIFO_enqueue(&readyQeue, OS_Control.currentTask);
		 }
	}
}

void OS_SVC(int *stackFrame)
{
	unsigned char svc_number;
	svc_number = *((unsigned char *)((unsigned char *)stackFrame[6] - 2));

	switch(svc_number) {
		case ActivateTask:
		case TerminateTask:
		case SuspendTask:
			updateSchedulerTable();
			if(OS_Control.OS_mode == OS_Running)
			{
				OS_decideWhatNext();
				OS_triggerPendsv();
			}
			break;
		case TaskWaitingTime:
			updateSchedulerTable();
	}

}

void MYRTOS_activateTask(mTaskHandle_t taskHandle)
{
	taskHandle->tskStatus = Waiting;
	OS_SVC_Set(ActivateTask);
}

void MYRTOS_suspendTask(mTaskHandle_t taskHandle)
{
	taskHandle->tskStatus = Suspended;
	OS_SVC_Set(SuspendTask);
}

void MYRTOS_terminateTask(mTaskHandle_t taskHandle)
{
	taskHandle->tskStatus = Suspended;
	OS_SVC_Set(SuspendTask);
}


void MYRTOS_delayTask(mTaskHandle_t taskHandle, unsigned int ticks_count)
{
	if(ticks_count == 0){
		return;
	}

	taskHandle->TimeWaiting.Blocking = Enable;
	taskHandle->TimeWaiting.ticks_count = ticks_count;
	taskHandle->tskStatus = Suspended;
	OS_SVC_Set(SuspendTask);
}

void MYRTOS_UpdateTasksWaitingTime()
{
	int i;
	for(i=0; i<noOfTasks; i++)
	{
		if(OS_Control.tasksHandle[i]->TimeWaiting.Blocking == Enable)
		{
			OS_Control.tasksHandle[i]->TimeWaiting.ticks_count--;
			if(OS_Control.tasksHandle[i]->TimeWaiting.ticks_count == 0)
			{
				OS_Control.tasksHandle[i]->TimeWaiting.Blocking = Disable;
				OS_Control.tasksHandle[i]->tskStatus = Waiting;
				OS_SVC_Set(TaskWaitingTime);
			}
		}
	}
}

mTaskHandle_t getCurrentTask()
{
	return OS_Control.currentTask;
}
