#pragma once

#include "stdint.h"

typedef struct
{
  uint8_t G;
  uint8_t R;
  uint8_t B;  
}GRB_u8_Color_TypeDef;

typedef struct
{
  float H;
  float S;
  float V;  
}HSV_f_Color_TypeDef;

typedef struct
{
  uint32_t G;
  uint32_t R;
  uint32_t B;
}
GRB_u32_LED_TypeDef;

void hsvf_to_grbu8(HSV_f_Color_TypeDef* hsv,GRB_u8_Color_TypeDef* rgb);
void rgb2LED(GRB_u8_Color_TypeDef* rgb, GRB_u32_LED_TypeDef* RGB);

