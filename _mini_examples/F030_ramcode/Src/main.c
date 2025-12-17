/**
  ******************************************************************************
  * @file    main.c
  * 
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private define ------------------------------------------------------------*/

#define _FlashSleep

/* Private variables ---------------------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

__attribute__((section(".ramcode")))
void Flash_Exit_From_Sleep(void);

__attribute__((section(".ramcode")))
void main_ram(void); 

void app_LPTIM1_Enable(void);

__attribute__((section(".ramcode")))
void app_LPTIM1_Delay(uint32_t ms);

__attribute__((section(".ramcode")))
void main_ram(void); 

__attribute__((section(".ramcode")))
void app_LPTIM1_Disable(void);


int main(void)
{
  /* Reset of all peripherals, Initializes the Systick. */
  
  /* System clock configuration */
  RCC->IOPENR |= RCC_IOPENR_GPIOAEN; 
  GPIOA->ODR = 0;

  // Impuls 1 configuration  PA1
  //  PA1  - output  pushpull - pull-up  
  GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE1_Msk) | (1 << GPIO_MODER_MODE1_Pos); 
  GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD1_Msk) | (2 << GPIO_PUPDR_PUPD1_Pos);
  
  app_LPTIM1_Enable();
  main_ram();
}

void main_ram(void) {
  // Flash sleep
  #ifdef _FlashSleep
    FLASH->STCR = 0x2800 | FLASH_STCR_SLEEP_EN;
  #endif
  
  // timeout 1 sec
  app_LPTIM1_Delay(2000);

  // set pin
  GPIOA->BSRR = GPIO_BSRR_BS1;
  // timeout 200 ms
  app_LPTIM1_Delay(200);
  // reset pin
  GPIOA->BRR = GPIO_BRR_BR1;

  // timeout 1 sec
  app_LPTIM1_Delay(2000);
  
  /* infinite loop */
  while (1) {
    GPIOA->BSRR = GPIO_BSRR_BS1;
    // timeout 300 ms
    app_LPTIM1_Delay(300);
    GPIOA->BRR = GPIO_BRR_BR1;
    app_LPTIM1_Delay(300);
  }
}

void app_LPTIM1_Enable(void) {
    // enable clock to LPTIM
    RCC->APBENR1 |= RCC_APBENR1_LPTIMEN;
    // LPTIM source LSI
    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_LPTIMSEL_Msk) | RCC_CCIPR_LPTIMSEL_0;
    // 
    LPTIM1->CFGR = LPTIM_CFGR_PRELOAD;
    // LPTIM enable
    LPTIM1->CR |= LPTIM_CR_ENABLE;
    // reset Counter for any read
    LPTIM1->CR |= LPTIM_CR_RSTARE; 
}

void app_LPTIM1_Disable(void) {
    // disable LPTIM
    LPTIM1->CR &= ~LPTIM_CR_ENABLE;
    // disable clock to LPTIM
    RCC->APBENR1 &= ~RCC_APBENR1_LPTIMEN;
}

__attribute__((section(".ramcode")))
void app_LPTIM1_Delay(uint32_t ms) {
    uint32_t Delay, temp; 

    Delay = (ms * 131U) >> 2;  // ~ 32768/1000
    // timer is 16 bit - more delay in loop
    while ( Delay > 60000U ) {
        // read for clear Counter 
        temp = LPTIM1->CNT;
        Delay -= 60000;      
        LPTIM1->ARR = 60000;
        // single start
        LPTIM1->CR |= LPTIM_CR_SNGSTRT; 
        // wait flag
        while ((LPTIM1->ISR & LPTIM_ICR_ARRMCF) == 0) {}
        // clear flag
        LPTIM1->ICR |= LPTIM_ICR_ARRMCF;
    }    
    // read for clear Counter 
    temp = LPTIM1->CNT;
    LPTIM1->ARR = Delay;
    // single start
    LPTIM1->CR |= LPTIM_CR_SNGSTRT; 
    // wait flag
    while ((LPTIM1->ISR & LPTIM_ICR_ARRMCF) == 0) {}
    // clear flag
    LPTIM1->ICR |= LPTIM_ICR_ARRMCF;
}

/* ***** END OF FILE ***************** */
