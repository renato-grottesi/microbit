#include <mbed.h>
#include "LCD_Driver.h"
#include "MicroBit.h"

LCD_Driver lcd;
DigitalIn bA(MICROBIT_PIN_P5);
DigitalIn bB(MICROBIT_PIN_P11);

int main(void)
{
	lcd.LCD_Init();
	lcd.LCD_Clear(LCD_COLOR::BLACK);

	lcd.LCD_ClearBuf();
	const char* text = "HELLO LCD";
	for(int i=0; i<128; i+=4)
	{
		lcd.LCD_DrawPoint(i, i, DOT_PIXEL::DOT_PIXEL_4, LCD_COLOR::GREEN);
	}
	for(int i=0; text[i]; ++i)
	{
		lcd.LCD_DisChar_1207(i*8, 64, 396+(text[i]-'A')*12, LCD_COLOR::RED);
	}
	lcd.LCD_Display();

	int x = 16;
	int y = 16;
	while(1)
	{
		lcd.LCD_DrawPoint(x, y, DOT_PIXEL::DOT_PIXEL_4, LCD_COLOR::WHITE);
		if(!bA.read()) x--;
		if(!bB.read()) x++;
		if(x>LCD_WIDTH-4) x = LCD_WIDTH-4;
		if(x<4) x = 4;
		lcd.LCD_DrawPoint(x, y, DOT_PIXEL::DOT_PIXEL_4, LCD_COLOR::BLUE);
		//lcd.LCD_Display();
		lcd.LCD_DisplayWindows(x-8, y-8, x+8, y+8);
	}

	return 0;
}
