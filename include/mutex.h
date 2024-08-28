/*
 * mutex.h
 *
 *  Created on: Aug 27, 2024
 *      Author: Abdelrahman
 */

#ifndef INCLUDE_MUTEX_H_
#define INCLUDE_MUTEX_H_

#include <stdlib.h>
#include "task.h"
#include "scheduler.h"

#define MAX_NUM_MUTEXES  3

typedef enum {
	MutexLockSucceed,
	MutexLockFailed,
	MutexReleaseSucceed,
	MutexReleaseFailed,
	MutexAlreadyLocked,
}MutexError_t;

typedef struct
{
	bool locked;
	mTaskHandle_t curr_locker_task;
	mTaskHandle_t next_locker_task;
}mMutex_t;

typedef mMutex_t* mMutexHandle_t;

mMutexHandle_t createMutex();
MutexError_t lockMutex(mMutexHandle_t mutexHandle);
MutexError_t releaseMutex(mMutexHandle_t mutexHandle);

#endif /* INCLUDE_MUTEX_H_ */
