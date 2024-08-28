/*
 * CortexMX_Porting.c
 *
 *  Created on: Aug 25, 2024
 *      Author: Abdelrahman
 */

#include "CortexMX_Porting.h"

extern void OS_decideWhatNext();
extern void MYRTOS_UpdateTasksWaitingTime();


void HW_init()
{
	// configure the systick timer for 1ms IRQ
	SysTick_Config(8000);

	// reduce the pendsv priority
	NVIC_SetPriority(PendSV_IRQn, 15);
}

__attribute ((naked)) void SVC_Handler(void)
{
	asm("tst lr , #4 \n\t"
		"ITE EQ \n\t"
		"mrseq r0, MSP \n\t"
		"mrsne r0, PSP \n\t"
		"b OS_SVC");
}

void OS_triggerPendsv()
{
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

unsigned int i =0;
void SysTick_Handler(void)
{
	MYRTOS_UpdateTasksWaitingTime();

	i ^= 1;
	OS_decideWhatNext();
	OS_triggerPendsv();
}

void HardFault_Handler()
{
	while(1);
}

void MemManage_Handler()
{
	while(1);
}

void BusFault_Handler()
{
	while(1);
}

void UsageFault_Handler()
{
	while(1);
}
