#include "ws2812.h"

uint8_t BUF_DMA [ARRAY_LEN+1] = {0};
HSV_f_Color_TypeDef led_hsv[LED_COUNT];
uint8_t BIT_LOW;
uint8_t BIT_HIGH;

void ws2812_pixel_rgb_to_buf_dma(GRB_u8_Color_TypeDef* grb, uint16_t posX)
    {
        uint32_t* ptr=(uint32_t*)grb;
        volatile uint16_t i;
        for (i=0;i<24;i++)
            {
                if (BitIsSet(*ptr,i))
                    BUF_DMA[posX*24+DELAY_LEN+i]=BIT_HIGH;
                else
                    BUF_DMA[posX*24+DELAY_LEN+i]=BIT_LOW;
            }
    }

void ws2812_init()
    {
        WS2812_GPIO_CLK_EN;                                             //Init GPIO Clocking
        
        WS2812_INIT_PIN;
        WS2812_TIM_CLK_EN;                                              //clock TIM    
        WS2812_TIM->PSC = 0;                                            //prescaler
        WS2812_TIM->ARR = (HAL_RCC_GetSysClockFreq()*1.25E-6)-2;        //restart register
        BIT_LOW = ((WS2812_TIM->ARR+2)*0.3)-1;
        BIT_HIGH = ((WS2812_TIM->ARR+2)*0.73);
	WS2812_TIM->BDTR |= TIM_BDTR_MOE;
        WS2812_TIM->CR1 |= TIM_CR1_ARPE;
        WS2812_INIT_TIM_CC;            
        RCC->AHBENR |= WS2812_DMA_CLK_EN;//clocking
        WS2812_DMA_Channel->CPAR = (uint32_t) & (WS2812_TIM_CCR);
        WS2812_DMA_Channel->CMAR = (uint32_t) & (BUF_DMA);
        WS2812_DMA_Channel->CCR =  DMA_CCR_DIR;                         //MemWS2812_TIM_CCR2Per
        WS2812_DMA_Channel->CCR |= DMA_CCR_MINC;                        //Memory inc mode
        WS2812_DMA_Channel->CCR |= DMA_CCR_PSIZE_0;                     //Periphery seze 16-bit
        WS2812_DMA_Channel->CCR |= DMA_CCR_CIRC;                        //circular mode on
        WS2812_DMA_Channel->CCR |= DMA_CCR_PL_1;                        //high priority
        WS2812_DMA_Channel->CCR |= DMA_CCR_TCIE;                        //Transfer complete interrupt enable
        HAL_NVIC_SetPriority(WS2812_DMA_Channel_IQRn, 0, 0);            //Setup priority DMNA Interrupts 
        NVIC_EnableIRQ (WS2812_DMA_Channel_IQRn);                       //Enable DMA Interrupts
        
        for (uint16_t i=DELAY_LEN;i<ARRAY_LEN-1;i++)
            BUF_DMA[i] = BIT_LOW;
        ws2812_update();
    }
void ws2812_update()
    {
        
        GRB_u8_Color_TypeDef led_grb;
        for (uint16_t i=0;i<LED_COUNT;i++)
            {
                hsvf_to_grbu8(&led_hsv[i],&led_grb);
                ws2812_pixel_rgb_to_buf_dma(&led_grb,i);
            }
        WS2812_TIM_SET_CCxDE;
        WS2812_DMA_Channel->CNDTR = ARRAY_LEN;
        WS2812_DMA_Channel->CCR |= DMA_CCR_EN;
        WS2812_TIM->CR1 |= TIM_CR1_CEN;
    }

void WS2812_DMA_IRQHandler()
    {
        
        WS2812_DMA->IFCR |= WS2812_DMA_IFCR_CTCIF;
        while(WS2812_TIM->SR & TIM_SR_UIF == 0){} 
        WS2812_TIM_RESET_CCxDE;
        WS2812_TIM_CCR=0;
        WS2812_TIM->CR1 &= ~TIM_CR1_CEN;
        WS2812_TIM->SR = ~TIM_SR_UIF;
    }

void set_all_led(float H,float S, float V)
    {
               for (uint16_t i=0;i<LED_COUNT;i++)
            {
                led_hsv[i].H=H;
                led_hsv[i].S=S;
                led_hsv[i].V=V;
            }
    }
