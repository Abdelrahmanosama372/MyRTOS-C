/*
 * mutex.c
 *
 *  Created on: Aug 27, 2024
 *      Author: Abdelrahman
 */

#include "mutex.h"

unsigned int mutexesCount = 0;
mMutex_t mutexes[MAX_NUM_MUTEXES];

mMutexHandle_t createMutex()
{
	if (mutexesCount >= MAX_NUM_MUTEXES)
	{
		return NULL;
	}

	mutexes[mutexesCount].locked = 0;
	mutexes[mutexesCount].curr_locker_task = NULL;
	mutexes[mutexesCount].next_locker_task = NULL;

	mutexesCount++;

	return &mutexes[mutexesCount - 1];
}

MutexError_t lockMutex(mMutexHandle_t mutexHandle)
{
	if (mutexHandle->curr_locker_task == NULL){
		mutexHandle->locked = 1;
		mutexHandle->curr_locker_task = getCurrentTask();
	}
	else if(mutexHandle->curr_locker_task == getCurrentTask())
	{
		return MutexAlreadyLocked;
	}
	else if(mutexHandle->locked == 1 && mutexHandle->next_locker_task == NULL)
	{
		mutexHandle->next_locker_task = getCurrentTask();
		MYRTOS_suspendTask(mutexHandle->next_locker_task);
	}
	else if(mutexHandle->locked == 1 && mutexHandle->next_locker_task != NULL)
	{
		return MutexLockFailed;
	}

	return MutexLockSucceed;
}

MutexError_t releaseMutex(mMutexHandle_t mutexHandle)
{
	if(getCurrentTask() != mutexHandle->curr_locker_task)
	{
		return MutexReleaseFailed;
	}

	mutexHandle->locked = 0;
	mutexHandle->curr_locker_task = mutexHandle->next_locker_task;
	mutexHandle->next_locker_task = NULL;

	if(mutexHandle->curr_locker_task != NULL)
	{
		MYRTOS_activateTask(mutexHandle->curr_locker_task);
	}

	return MutexReleaseSucceed;
}
