#include <mbed.h>
#include "LCD_Driver.h"
#include "MicroBit.h"
#include <us_ticker_api.h>

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define MARGIN 8

LCD_Driver lcd;

int x;
int y;

static void draw_float(int delta_us)
{
	// 4 floating point variables
	static float flx = 16.0;
	static float fly = 16.0;
	static float speed_x = 23.0;
	static float speed_y = 17.0;

	float delta_sec = ((float)delta_us /1000000.0);

	flx += speed_x * delta_sec;
	fly += speed_y * delta_sec;

	x = flx;
	y = fly;

	if(x>LCD_WIDTH-MARGIN  || x<MARGIN) speed_x = -speed_x;
	if(y>LCD_HEIGHT-MARGIN || y<MARGIN) speed_y = -speed_y;
}

static void draw_fixed(int delta_us)
{
	// 4 fixed precision variables
	static int fxx = 16<<7;
	static int fxy = 16<<7;
	static int speed_x = 12;
	static int speed_y = 9;

	// (1/(2^7))* (1000000/(2^12)) = 1.9 pixel per second resolution
	int delta_fixed = delta_us>>12;

	fxx += speed_x * delta_fixed;
	fxy += speed_y * delta_fixed;

	x = fxx>>7;
	y = fxy>>7;

	if(x>LCD_WIDTH-MARGIN  || x<MARGIN) speed_x = -speed_x;
	if(y>LCD_HEIGHT-MARGIN || y<MARGIN) speed_y = -speed_y;
}

int main(void)
{
	us_ticker_init();

	lcd.LCD_Init();
	lcd.LCD_Clear(LCD_COLOR::BLACK);

	lcd.LCD_ClearBuf();
	lcd.LCD_Display();

	uint32_t old_time = us_ticker_read();
	while(1)
	{
		uint32_t new_time = us_ticker_read();
		uint32_t delta_us = new_time - old_time;
		old_time = new_time;

		lcd.LCD_DrawPoint(x, y, DOT_PIXEL::DOT_PIXEL_4, LCD_COLOR::WHITE);
		//draw_float(delta_us);
		draw_fixed(delta_us);
		x = CLAMP(x, MARGIN, LCD_WIDTH-MARGIN);
		y = CLAMP(y, MARGIN, LCD_HEIGHT-MARGIN);
		lcd.LCD_DrawPoint(x, y, DOT_PIXEL::DOT_PIXEL_4, LCD_COLOR::RED);
		lcd.LCD_DisplayWindows(x-8, y-8, x+8, y+8);
	}

	return 0;
}
