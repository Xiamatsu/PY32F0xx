/**
  ******************************************************************************
  * @file    main.c
  *          Пример работы с низким потреблением
  *          при запуске в startup сразу HSI = ~2.1 MHz (300 мкА)
  *          в system_init переход на LSI с частотой - 19.2 kHz (190мкА)
  *          настроен  MCO - PB1
  *          выход для импульсов - PB0 
  *          в цикле
  *              задержка ~5 сек
  *              переход Flash in sleep
  *              10 импульсов
  *              задержка ~5 сек
  *              выход Flash из sleep
  *              5 импульсов чуть более широких
  *                в начало цикла
  * 
  *  потребление если порты включены и на PB1 PB0 нет нагрузки
  *    ~190 мкА  когда Flash sleep - off
  *    ~130 мкА  когда Flash sleep - on
  *
  *  потребление если порты вsключены 
  *    ~160 мкА  когда Flash sleep - off
  *    ~100 мкА  когда Flash sleep - on
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void DelayX(uint32_t mdelay);

void FlashExitFromSleep(void);


int main(void)
{
  uint32_t count;  
  /* Reset of all peripherals, Initializes the Systick. */
  
  /* System clock configuration */

  // MCO configuration  PB1
  RCC->CFGR |= RCC_CFGR_MCOSEL_0;
  
  RCC->IOPENR |= RCC_IOPENR_GPIOBEN; 
  
  //  PB1 for MCO - output  pushpull - pull-up  
  GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE1_Msk) | (2 << GPIO_MODER_MODE1_Pos); 
  GPIOB->OSPEEDR = (GPIOB->OSPEEDR & ~GPIO_OSPEEDR_OSPEED1_Msk) | (3 << GPIO_OSPEEDR_OSPEED1_Pos);
  GPIOB->PUPDR = (GPIOB->PUPDR & ~GPIO_PUPDR_PUPD1_Msk) | (1 << GPIO_PUPDR_PUPD1_Pos);
  GPIOB->AFR[0] = (GPIOB->AFR[0] & ~GPIO_AFRL_AFSEL1_Msk) | (4 << GPIO_AFRL_AFSEL1_Pos);

  // add io  PB0  - output  pushpull - no pull-up no pull-down 
  GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE0_Msk) | (1 << GPIO_MODER_MODE0_Pos); 
  //GPIOB->OSPEEDR = (GPIOB->OSPEEDR & ~GPIO_OSPEEDR_OSPEED0_Msk) | (0 << GPIO_OSPEEDR_OSPEED0_Pos);
  //GPIOB->PUPDR = (GPIOB->PUPDR & ~GPIO_PUPDR_PUPD0_Msk) | (0 << GPIO_PUPDR_PUPD0_Pos);

  
  /* infinite loop */
  while (1)
  {
    /* delay  ~5c  */
    DelayX(5000);
    
    /*  Flash Sleep Mode with constant from Factory Config */
    FLASH->STCR = 0x2800 | FLASH_STCR_SLEEP_EN;
    
    /*  10 impulses on PB0 */
    for (count = 0; count < 10; count++) {
        GPIOB->BSRR |= GPIO_BSRR_BS0;
        __NOP();
        GPIOB->BRR |= GPIO_BRR_BR0;
    }

    /* delay  ~5c  */
    DelayX(5000);

    FlashExitFromSleep();

    /*  5 impulses on PB0  */
    for (count = 0; count < 5; count++) {
        GPIOB->BSRR |= GPIO_BSRR_BS0;
        __NOP();
        __NOP();
        GPIOB->BRR |= GPIO_BRR_BR0;
    }
  }
}

static void DelayX(uint32_t mdelay)
{
  __IO uint32_t Delay = (mdelay * 19200U) / 7U / 1000U;
  do
  {
    __NOP();
  }
  while (Delay --);
}

/* ***** END OF FILE ***************** */
