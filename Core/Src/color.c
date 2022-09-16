#include "color.h"

void hsvf_to_grbu8(HSV_f_Color_TypeDef* hsv,GRB_u8_Color_TypeDef* rgb)
{
	uint8_t R,G,B;
	if(hsv->H>1.0)
		hsv->H=1.0;
	if(hsv->V>1.0)
		hsv->V=1.0;
	if(hsv->S>1.0)
		hsv->S=1.0;
	
	uint8_t Hi = (uint8_t)(hsv->H*6.0);
	float ff = hsv->H*6.0-(float)Hi;
	float p = hsv->V*(1.0-hsv->S);
	float q = hsv->V*(1.0-hsv->S*ff);
	float t = hsv->V*(1.0-hsv->S*(1.0-ff));
	
	float Vinc=255.0*t;
	float Vdec=255.0*q;
	float Vmin=255.0*p;
	float V=hsv->V*255.0;
	
	switch(Hi)
	{
	case 0:
		R=V;G=Vinc;B=Vmin;
		break;
	case 1:
		R=Vdec;G=V;B=Vmin;
		break;
	case 2:
		R=Vmin;G=V;B=Vinc;
		break;
	case 3:
		R=Vmin;G=Vdec;B=V;
		break;
	case 4:
		R=Vinc;G=Vmin;B=V;
		break;
	case 5:
		R=V;G=Vmin;B=Vdec;
		break;
	}
	
	 rgb->R = R;
	 rgb->G = G;
	 rgb->B = B;
}

void rgb2LED(GRB_u8_Color_TypeDef* rgb, GRB_u32_LED_TypeDef* RGB)
{

}