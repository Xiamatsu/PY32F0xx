/**
  ******************************************************************************
  * @file    main.c
  * 
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private define ------------------------------------------------------------*/

//#define _FlashSleep

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
  
  //  PA1  - Alternate MCO output  pushpull - no pull-up no pull-down  
  GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE1_Msk) | (2 << GPIO_MODER_MODE1_Pos); 
  GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD1_Msk) | (2 << GPIO_PUPDR_PUPD1_Pos);
  GPIOA->OSPEEDR = (GPIOA->OSPEEDR & ~GPIO_OSPEEDR_OSPEED1_Msk) | (3 << GPIO_OSPEEDR_OSPEED1_Pos);
  GPIOA->AFR[0] = (GPIOA->AFR[0] & ~GPIO_AFRL_AFSEL1_Msk) | (15 << GPIO_AFRL_AFSEL1_Pos);
  // MCO out SYSCLK/2  
  //RCC->CFGR = (RCC->CFGR & 0x00FFFFFF) | 0x11000000;
  // MCO out SYSCLK/4  
  RCC->CFGR = (RCC->CFGR & 0x00FFFFFF) | 0x21000000;

  app_LPTIM1_Enable();
  main_ram();
}

void main_ram(void) {
  uint32_t  k1, k2, k3; 
  app_LPTIM1_Delay(6000);

  // константы
  // k1 - HSITRIM_L 
  // k2 - HSITRIM_H
  // k3 - HSI_FS 
  
  //  перебор для HSITRIM = 0x0001 - 0x1FFF 
  //  максимальные частоты
  k1 = 1; k2 = 15; 
  k3 = 4;    // Base HSI for 24 MHz        
  RCC->ICSCR = (RCC->ICSCR & 0xFFFF0000) | (k3 << 13) | (k2 << 9) | k1;
  while ( READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0 ) {}; 
  /* source HSI for PLL  */
  RCC->PLLCFGR = 0;		
  /* PLL on */
  SET_BIT(RCC->CR, RCC_CR_PLLON);
  /* ready PLL */
  while ( READ_BIT(RCC->CR, RCC_CR_PLLRDY) != 0 ) {}; 

  /* set PLL as system clock  HCLK = PCLK = 48MHz */
  RCC->CFGR = (RCC->CFGR & 0xFFFF0000) | 0x00000002;
  /* ready PLL as SystemClock */
  while ( (RCC->CFGR & RCC_CFGR_SWS_Msk ) != 0b010000 ) {}; 
  
  // основной цикл перебора частот HSI
  while (1) {
    // перебор частот по HSITRIM_L
    for ( k1 = 1; k1 < 0x200; k1 += 34 ) {
        RCC->ICSCR = (RCC->ICSCR & 0xFFFF0000) | (k3 << 13) | (k2 << 9) | k1;
        app_LPTIM1_Delay(4000);
    }
    // перебор частот по HSITRIM_H
    //k2 = (k2 == 0xF ) ? 12 : k2 + 1; 
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
