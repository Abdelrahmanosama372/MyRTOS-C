/*
 * CortexMX_Porting.h
 *
 *  Created on: Aug 25, 2024
 *      Author: Abdelrahman
 */

#ifndef INCLUDE_CORTEXMX_PORTING_H_
#define INCLUDE_CORTEXMX_PORTING_H_

#include "ARMCM3.h"
#include "scheduler.h"

#define OS_GET_PSP(add)  __asm volatile("MRS R0, PSP \t\n MOV %0, R0": "=r"(add))
#define OS_SET_PSP(add)  __asm volatile("MOV R0, %0 \t\n MSR PSP, R0": : "r"(add))

#define OS_SWITCH_SP_TO_PSP __asm volatile("MRS R0, CONTROL \t\n ORR R0, #0x2 \t\n MSR CONTROL, R0");
#define OS_SWITCH_SP_TO_MSP __asm volatile("MRS R0, CONTROL \t\n AND R0, #0x5 \t\n MSR CONTROL, R0");



void HW_init();
void OS_triggerPendsv();

#endif /* INCLUDE_CORTEXMX_PORTING_H_ */
