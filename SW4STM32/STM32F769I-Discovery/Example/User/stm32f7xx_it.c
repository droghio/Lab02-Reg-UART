#include "stm32f7xx_it.h"

// Nice place for the IRQs
//  Normally if they trigger you wouldn't be able to tell them apart since the all redirect to the same function.
//  Here we specify unique functions for all of them so it is easier to track.

void USART1_IRQHandler(){
	asm("nop");
	while (1);
	return;
}

void HardFault_Handler() {
	asm("nop");
	while (1);
	return;
}

void WWDG_IRQHandler(){
	asm("nop");
	while (1);
	return;
}

void USART6_IRQHandler() {
	asm("nop");
	while (1);
	return;
}

void USART3_IRQHandler() {
	asm("nop");
	while (1);
	return;
}

void USART2_IRQHandler() {
	asm("nop");
	while (1);
	return;
}

void UART8_IRQHandler() {
	asm("nop");
	while (1);
	return;
}

void UART7_IRQHandler() {
	asm("nop");
	while (1);
	return;
}

void UART5_IRQHandler() {
	asm("nop");
	while (1);
	return;
}

void UART4_IRQHandler() {
	asm("nop");
	while (1);
	return;
}

void UsageFault_Handler() {
	asm("nop");
	while (1);
	return;
}

void NMI_Handler(void) {
	asm("nop");
	while (1);
	return;
}
void MemManage_Handler(void) {
	asm("nop");
	while (1);
	return;
}
void BusFault_Handler(void) {
	asm("nop");
	while (1);
	return;
}
void SVC_Handler(void) {
	asm("nop");
	while (1);
	return;
}
void DebugMon_Handler(void) {
	asm("nop");
	while (1);
	return;
}
void PendSV_Handler(void) {
	asm("nop");
	while (1);
	return;
}
