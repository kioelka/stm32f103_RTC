#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
    
typedef struct
    {
        uint8_t h;
        uint8_t m;
        uint8_t s;
        uint32_t rawSec;
    }timeStruct;

void Error_Handler(void);
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_USART2_UART_Init(void);
void sendTime(timeStruct*);
void applytime (uint8_t*);
void sendStr(const uint8_t*);


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
