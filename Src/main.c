//--------------------------
// Lab 2 - Timer Interrupts
//--------------------------
// Objective:
//   Build a small game that records user's reaction time.
//
// Progress:
//   Timer and interrupts are working, still need to build
//   game mechanics. LED 2 will blink on and off every 100ms.
//   LED 1 is controlled via the push button.
//---------------------------
// Authors:
//   Mark Blanco
//   John Drogo
//

//
//
// -- Imports ---------------
//
#include <stdio.h>
#include <stdlib.h>

// CMSIS Header, defines register structure.
#include "stm32f769xx.h"
// CMSIS Header, defines SystemInit function.
#include "system_stm32f7xx.h"

// Init HAL for UART
#include "stm32f7xx_hal_conf.h"
#include "vcp.h"

//
//
// -- Function Defines ------
//
void SystemInit();
void Init_Timer();
void Init_GPIO();

void blinkScreen();
int _write(int file, char *ptr, unsigned int len) {
	vcp_print(ptr, len);
	return len;
}

void TIM6_DAC_IRQHandler();
void EXTI0_IRQHandler();
void SysTick_Handler(void);

//
//
// -- Code Body -------------
//
volatile uint8_t timeUpdated = 0;
volatile uint8_t buttonPressed = 0;
volatile uint32_t elapsed = 0;

int32_t randomNumber = 0;
uint32_t startTime = 0;
float averageScore = 0;
unsigned int iterations = 0;

int main() {
	//SystemInit(); <- Called by assembly startup file.
	HAL_Init();
	vcp_init();
	Init_Timer();
	Init_GPIO();

	// Setup random number generation.
	srand(0);

	blinkScreen();
	printf("\033[2J\033[;H");
	printf("\rHowdy all!\r\n");

	printf("\r\n\nWelcome to the game of the century.\r\nWatch the screen, press the button, win everything.\r\n");
	printf("\r\nPress the button to begin. If you dare.\r\n\n");

	// Wait until user pressed the button.
	buttonPressed = 0;
	while(!buttonPressed);
	buttonPressed = 0;

	randomNumber = elapsed + (rand()%100) + 5;
	while (1) {
		if (timeUpdated) {
			if (randomNumber-elapsed > 0){
				printf("\033[2J\033[;H");
				printf("\rWatch carefully, you never know when I'll go!\r\n");
			} else {
				startTime = elapsed;
				blinkScreen();

				// Wait until user pressed the button.
				buttonPressed = 0;
				while(!buttonPressed);
				buttonPressed = 0;

				uint32_t trialTime = elapsed - startTime;
				averageScore = ((averageScore*iterations) + trialTime)/++iterations;

				printf("\033[2J\033[;H");
				printf("\r%u00ms not bad, your average reaction time is %d.%ds across %u tries.\n\rHit the button to try again!\r\n",
						(unsigned int) trialTime,
						(int)averageScore/10,
						(int)((averageScore/10-((int)averageScore/10))*100), iterations);
				fflush(stdout);

				// Wait until user pressed the button.
				buttonPressed = 0;
				while(!buttonPressed);
				buttonPressed = 0;

				// Generate the next number.
				randomNumber = elapsed + (rand()%100) + 5;
			}
			timeUpdated = 0;
		}
	}
}

//
//
// -- Utility Functions ------
//
void blinkScreen(){
	printf("\033[30;47m");
	// Clear and redraw display (flash it).
	printf("\a\033[s\033[2J\033[u");
	fflush(stdout);
	HAL_Delay(100);

	printf("\033[37;40m");
	// Clear and redraw display (flash it).
	printf("\033[s\033[2J\033[u");
	fflush(stdout);
}


//
//
// -- Init Functions ----------
//
void SystemInit() {
	// Enable FPU, set CP10 and CP11 Full Access
	SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));
	// Reset the RCC clock configuration
	// Set HSION bit
	RCC->CR |= (uint32_t) 0x00000001;
	// Reset CFGR register
	RCC->CFGR = 0x00000000;
	// Reset HSEON, CSSON and PLLON bits
	RCC->CR &= (uint32_t) 0xFEF6FFFF;
	// Reset PLLCFGR register
	RCC->PLLCFGR = 0x24003010;
	// Reset HSEBYP bit
	RCC->CR &= (uint32_t) 0xFFFBFFFF;
	// Disable all interrupts
	RCC->CIR = 0x00000000;
	// Vector Table Relocation in Internal FLASH
	SCB->VTOR = FLASH_BASE;
}

void Init_Timer() {
	// Enable the TIM6 interrupt.
	// Looks like HAL hid this little gem, this register isn't mentioned in
	//   the STM32F7 ARM Reference Manual....
	NVIC->ISER[TIM6_DAC_IRQn / 32U] = (uint32_t) (1UL
			<< (((uint32_t) TIM6_DAC_IRQn) & 0x1FUL));

	// Enable TIM6 clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	asm ( "nop" );
	asm ( "nop" );

	// Set pre-scaler to make a 100kHz ticker.
	TIM6->PSC = (uint32_t) ((SystemCoreClock / 100000U) - 1U);

	// Set the Auto-reload Value for 10Hz overflow
	TIM6->ARR = (100000U / 10U) - 1U;

	// Enable Update Interrupts.
	TIM6->EGR = TIM_EGR_UG;

	TIM6->DIER |= TIM_DIER_UIE;
	TIM6->CR1 |= TIM_CR1_CEN;
}

void Init_GPIO() {
	// Enable GPIO clocks?
	// Looks like GPIO reg updates are synced to a base clock.
	//  for any changes to appear the clocks need to be running.
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOJEN;
	// Delay after an RCC peripheral clock enabling
	asm ("nop");
	asm ("nop");

	// Set Pin 13 to output. LED1
	GPIOJ->MODER |= GPIO_MODER_MODER13
			& (GPIO_MODER_MODER13 - (GPIO_MODER_MODER13 >> 1));

	// Set Pin 5 to output. LED2
	GPIOJ->MODER |= GPIO_MODER_MODER5
			& (GPIO_MODER_MODER5 - (GPIO_MODER_MODER5 >> 1));

	// GPIO Interrupt
	// By default pin 0 will trigger the interrupt,
	//  so no need to mess with SYSCFG_EXTICR1.

	// Set Pin 0 as input (button) with pull-up.
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0
			& (GPIO_PUPDR_PUPDR0 - (GPIO_PUPDR_PUPDR0 >> 1));

	// Set interrupt enable for EXTI0.
	NVIC->ISER[EXTI0_IRQn / 32U] = (uint32_t) (1UL
			<< (((uint32_t) EXTI0_IRQn) & 0x1FUL));

	// Unmask interrupt.
	EXTI->IMR |= EXTI_IMR_MR0;

	// Register for rising edge.
	EXTI->RTSR |= EXTI_RTSR_TR0;
}

//
//
// -- ISRs (IRQs) -------------
//
void TIM6_DAC_IRQHandler() {
	// Clear Interrupt Bit
	TIM6->SR = ~TIM_DIER_UIE;

	//Toggle GPIO_PIN_13 (LED1)
	GPIOJ->ODR ^= ((uint16_t) 0x0020U);

	// Updated variable to print update.
	elapsed++;
	timeUpdated = 1;
}

void EXTI0_IRQHandler() {
	// Clear Interrupt Bit (by setting it, weird I know).
	EXTI->PR |= EXTI_PR_PR0;

	//Toggle GPIO_PIN_5 (LED2)
	GPIOJ->ODR ^= ((uint16_t) 0x2000U);

	buttonPressed = 1;
}

void SysTick_Handler(void) {
	HAL_IncTick();
	return;
}
