#include <mbed.h>
#include "LCD_Driver.h"
#include "MicroBit.h"
#include <us_ticker_api.h>
#include "bitmaps.h"

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define MARGIN 4

LCD_Driver lcd;
DigitalIn bA(MICROBIT_PIN_P5);
DigitalIn bB(MICROBIT_PIN_P11);

struct s_moving
{
	// current position integer
	uint16_t x, y;
	// old position integer
	uint16_t ox, oy;
	// current position fixed precision
	int16_t fx, fy;
	// current speed fixed precision
	int16_t sx, sy;
};

s_moving paddle;
s_moving ball;

void init_s_moving(s_moving* m, uint16_t x, uint16_t y, uint16_t sx, uint16_t sy)
{
	m->x = x;
	m->y = y;
	m->ox = x;
	m->oy = y;
	m->fx = x<<7;
	m->fy = y<<7;
	m->sx = sx;
	m->sy = sy;
}

uint8_t bricks[8][8] =
{
	{1, 1, 1, 0, 0, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 0, 0, 1, 1, 1},
	{1, 1, 1, 0, 0, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1},
	{0, 1, 1, 1, 1, 1, 1, 0},
	{0, 0, 1, 0, 0, 1, 0, 0},
};

static void update_ball(int delta_us)
{
	// (1/(2^7))* (1000000/(2^12)) = 1.9 pixel per second resolution
	int delta_fixed = delta_us>>12;

	ball.fx += ball.sx * delta_fixed;
	ball.fy += ball.sy * delta_fixed;

	ball.ox = ball.x;
	ball.oy = ball.y;

	ball.x = ball.fx>>7;
	ball.y = ball.fy>>7;

	if(ball.x>LCD_WIDTH-MARGIN  || ball.x<MARGIN) ball.sx = -ball.sx;
	if(ball.y>LCD_HEIGHT-MARGIN || ball.y<MARGIN) ball.sy = -ball.sy;

	ball.x = CLAMP(ball.x, MARGIN, LCD_WIDTH-MARGIN);
	ball.y = CLAMP(ball.y, MARGIN, LCD_HEIGHT-MARGIN);
}

static void update_paddle(int delta_us)
{
	// (1/(2^7))* (1000000/(2^12)) = 1.9 pixel per second resolution
	int delta_fixed = delta_us>>12;

	if(paddle.x>18 && !bA.read()) paddle.fx -= paddle.sx * delta_fixed;
	if(paddle.x<LCD_WIDTH-18 && !bB.read()) paddle.fx += paddle.sx * delta_fixed;

	paddle.x = paddle.fx>>7;
	paddle.y = paddle.fy>>7;

	paddle.x = CLAMP(paddle.x, 18, LCD_WIDTH-18);
}

static void draw_background(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	for(uint16_t col = x0; col < x1; col++)
	{
		for(uint16_t row = y0; row < y1; row++)
		{
			lcd.LCD_SetPoint(col, row, back_bp[row%8][col%8]);
		}
	}
}

static void collide()
{
	if(ball.y<14 && ball.sy<0)
	{
		// Collide with paddle
		if(ball.x+2<paddle.x-16 || ball.x-2>paddle.x+16)
		{
			// Lose a life
			init_s_moving(&ball, 16, 10, 12, 9);
		}
		else
		{
			ball.sy = -ball.sy;
			//ball.sx = -ball.sx;
		}
	}

	if(ball.y>8*6-4)
	{
		int bx = (ball.x>>4)-1; if (bx<1) bx=1; if (bx>6) bx=6;
		int by = (ball.y>>3)-6; if (by<1) by=1; if (by>6) by=6;
		// Collide with bricks
		for(int c=bx-1; c<bx+2; c++)
		{
			uint16_t x = (c+1)*16;
			for(int r=by-1; r<by+2; r++)
			{
				if(bricks[r][c])
				{
					uint16_t y = (r+6)*8;

					uint8_t h_collision = (ball.y+2>y && ball.y+2<y+8) || (ball.y-2>y && ball.y-2<y+8);
					uint8_t v_collision = (ball.x+2>x && ball.x+2<x+16) || (ball.x-2>x && ball.x-2<x+16);
					if(h_collision && v_collision)
					{
						bricks[r][c] = 0;
						draw_background(x, y, x+16, y+8);
						lcd.LCD_DisplayWindows(x, y, x+16, y+8);

						uint8_t ho_collision = (ball.oy+2>y && ball.oy+2<y+8) || (ball.oy-2>y && ball.oy-2<y+8);
						uint8_t vo_collision = (ball.ox+2>x && ball.ox+2<x+16) || (ball.ox-2>x && ball.ox-2<x+16);
						if(!ho_collision) ball.sy = -ball.sy;
						if(!vo_collision) ball.sx = -ball.sx;
					}
				}
			}
		}
	}
}

static void draw_bricks(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	for(uint16_t col = x0; col < x1; col++)
	{
		for(uint16_t row = y0; row < y1; row++)
		{
			if(col>(16-1) && row>(8*6-1) && col<(16*9-1) && row<(8*14-1))
			{
				uint16_t c=(col-16)/16;
				uint16_t r=(row-8*6)/8;
				if(bricks[r][c]) lcd.LCD_SetPoint(col, row, brick_bp[row%8][col%16]);
			}
		}
	}
}

static void draw_ball(uint16_t x0, uint16_t y0)
{
	for(uint16_t col = 0; col < 4; col++)
	{
		for(uint16_t row = 0; row < 4; row++)
		{
			if(ball_bp[row][col]!=transparent_color)
			{
				lcd.LCD_SetPoint(col+x0, row+y0, ball_bp[row][col]);
			}
		}
	}
}

static void draw_paddle(uint16_t x0, uint16_t y0)
{
	for(uint16_t col = 0; col < 32; col++)
	{
		for(uint16_t row = 0; row < 8; row++)
		{
			if(paddle_bp[row][col]!=transparent_color)
			{
				lcd.LCD_SetPoint(col+x0, row+y0, paddle_bp[row][col]);
			}
		}
	}
}

int main(void)
{
	init_s_moving(&ball, 16, 10, 12, 9);
	init_s_moving(&paddle, 80, 8, 16, 0);

	us_ticker_init();

	lcd.LCD_Init();
	lcd.LCD_Clear(LCD_COLOR::BLACK);

	lcd.LCD_ClearBuf();
	draw_background(0, 0, LCD_WIDTH, LCD_HEIGHT);
	draw_bricks(0, 0, LCD_WIDTH, LCD_HEIGHT);
	lcd.LCD_Display();

	uint32_t old_time = us_ticker_read();
	while(1)
	{
		uint32_t new_time = us_ticker_read();
		uint32_t delta_us = new_time - old_time;
		old_time = new_time;

		collide();

		draw_background(ball.x-2, ball.y-2, ball.x+2, ball.y+2);
		draw_bricks(ball.x-2, ball.y-2, ball.x+2, ball.y+2);
		update_ball(delta_us);
		draw_ball(ball.x-2, ball.y-2);
		lcd.LCD_DisplayWindows(ball.x-4, ball.y-4, ball.x+4, ball.y+4);

		draw_background(paddle.x-16, paddle.y-4, paddle.x+16, paddle.y+4);
		update_paddle(delta_us);
		draw_paddle(paddle.x-16, paddle.y-4);
		lcd.LCD_DisplayWindows(paddle.x-18, paddle.y-4, paddle.x+18, paddle.y+8);
	}

	return 0;
}
