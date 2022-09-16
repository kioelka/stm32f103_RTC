#include "main.h"
#define UART_BUF_SIZE 16

#ifdef __cplusplus
extern "C" {
#endif



const uint32_t goodmorning = 8*3600;
const uint32_t goodnight = 23*3600;

RTC_HandleTypeDef hrtc;
UART_HandleTypeDef huart2;
timeStruct t;
int calibr_sec;

char report[16] = "0";
uint8_t UART_buffer[UART_BUF_SIZE];


int main(void)
    {
        
        HAL_Init();
        
        
        SystemClock_Config();
        
        MX_GPIO_Init();
        MX_RTC_Init();
        MX_USART2_UART_Init();
        
        ws2812_init();
            
        while (1)
            {
                ws2812_update();
            }
    }


void SystemClock_Config(void)
    {
        RCC_OscInitTypeDef RCC_OscInitStruct = {0};
        RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
        RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
        
        /** Initializes the RCC Oscillators according to the specified parameters
        * in the RCC_OscInitTypeDef structure.
        */
        RCC->BDCR |= RCC_BDCR_LSEON;
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState = RCC_HSE_ON;
        RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
        RCC_OscInitStruct.HSIState = RCC_HSI_ON;
        RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
            {
                Error_Handler();
            }
        /** Initializes the CPU, AHB and APB buses clocks
        */
        RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
            |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
        RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
        
        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
            {
                Error_Handler();
            }
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
        PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV128;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
            {
                Error_Handler();
            }
    }
void sendTime(timeStruct *d)
    {
        sprintf(report,"Time: %d:%d:%d\r\n",d->h,d->m,d->s);
        printf("Time: %d:%d:%d\r\n",d->h,d->m,d->s);
        for (char *x = report; *x!='\0'; x++)
            {
                while (!(USART2->SR & USART_SR_TC));
                USART2->DR = *x;  
            }
    }
void sendStr(const uint8_t *d)
    {
        for ( ;*d!='\0'; d++)
            {
                while (!(USART2->SR & USART_SR_TC));
                USART2->DR = *d;  
            }
        
    }


static void MX_RTC_Init(void)
    {

        hrtc.Instance = RTC;
        hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
        hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
        RTC->CRH |= RTC_CRH_SECIE;
        NVIC_EnableIRQ(RTC_IRQn);
        if (HAL_RTC_Init(&hrtc) != HAL_OK)
            {
                Error_Handler();
            }
        
    }
void RTC_IRQHandler()
    {
        t.rawSec = (RTC->CNTL + calibr_sec) % (60*60*24);
        t.s = t.rawSec % 60;
        t.m = (t.rawSec / 60) % 60;
        t.h = (t.rawSec / 3600);
        sendTime(&t);
        
        if (t.rawSec == goodmorning)
            {
                GPIOA->BSRR = GPIO_BSRR_BS0;
                sendStr("Good morning!");
            }
        if (t.rawSec == goodnight)
            {
                GPIOA->BSRR = GPIO_BSRR_BR0;
                sendStr("Good night!");
            }
        
        RTC->CRL &= ~RTC_CRL_SECF; // сбросить флаг
    }

static void MX_USART2_UART_Init(void)
    {
        huart2.Instance = USART2;
        huart2.Init.BaudRate = 9600;
        huart2.Init.WordLength = UART_WORDLENGTH_8B;
        huart2.Init.StopBits = UART_STOPBITS_1;
        huart2.Init.Parity = UART_PARITY_NONE;
        huart2.Init.Mode = UART_MODE_TX_RX;
        huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        huart2.Init.OverSampling = UART_OVERSAMPLING_16;
        
        if (HAL_UART_Init(&huart2) != HAL_OK)
            {
                Error_Handler();
            }
        USART2->CR1 |= USART_CR1_RXNEIE | USART_CR1_IDLEIE;
        NVIC_EnableIRQ(USART2_IRQn);//enable USART1 and interrupts

        
    }

void USART2_IRQHandler()
    {
        static uint8_t cnt;
        if (USART2->SR & USART_SR_RXNE)
            {
                UART_buffer[cnt++] = USART2->DR;
                USART2->SR &= ~USART_SR_RXNE;
                return;
            }
        else
            {
                if (cnt>0)
                    {
                        applytime(UART_buffer);
                        for (uint8_t x=cnt;x<UART_BUF_SIZE-1;x++)
                            UART_buffer[x] = 0;
                        cnt = 0;
                    }
                char v0id = (USART2->DR);//reset IDLE bit
            }
         
    }
void applytime (uint8_t *newtime)
    {
        if (!(*(uint32_t*)newtime))
            {
                sendStr("Illegal time\r");
                return;
            }
        uint8_t h,m,s;
        if (*newtime=='t')
            {
                h = atoi(newtime+1);
                m = atoi(newtime+4-(h<10));
                s = atoi(newtime+7-(h<10)-(m<10));
            }
        else
            {
        h = *(newtime++);
        m = *(newtime++);
        s = *newtime;
            }
        unsigned raw = s+m*60+h*3600;
        calibr_sec = raw - RTC->CNTL; 
        sendStr("New time applied\r\n");
        //printf("new time: %d:%d:%d\r\n",h,m,s);
        if (h > 8 && h < 23)
            GPIOA->BSRR = GPIO_BSRR_BS0;
        else
            GPIOA->BSRR = GPIO_BSRR_BR0;
    }


static void MX_GPIO_Init(void)
    {
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        
        /* GPIO Ports Clock Enable */
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        
        /*Configure GPIO pins : PC13 PC14 PC15 */
        GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        
        /*Configure GPIO pins : PA0 PA1 PA4 PA5
        PA6 PA7 PA8 PA9
        PA10 PA11 PA12 PA15 */
        GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5
            |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9
                |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        /*Configure GPIO pins : PB0 PB1 PB2 PB10
        PB11 PB12 PB13 PB14
        PB15 PB3 PB4 PB5
        PB6 PB7 PB8 PB9 */
        GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
            |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                |GPIO_PIN_15|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                    |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        //PIN A0 OUTPUT
        GPIOA->CRL = 0x00404202;
        
    }

void Error_Handler(void)
    {
        __disable_irq();
        while (1)
            {
            }
    }

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
    {
    
    }
#endif 

#ifdef __cplusplus
}
#endif