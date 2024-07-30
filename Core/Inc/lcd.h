#pragma once

#include <stdint.h>

#define BLACK			0x0000
#define RED			0x00f8
#define GREEN			0xe007
#define BLUE			0x1f00
#define YELLOW			0xe0ff
#define MAGENTA			0x1ff8
#define CYAN			0xff07
#define WHITE			0xffff

#define LCD_WIDTH 100
#define LCD_HEIGHT 80

struct Lcd{
	uint16_t frameBuffer[80*100];
};

void initLcd();

void lcdCreateRectangle(int x, int y, int width, int height, uint16_t color);

void lcdColorPixel(int x, int y, uint16_t color);

void lcd_draw_image(int x, int y, int width, int height, const uint8_t* data);

void show();
void putPixel(int x, int y, uint16_t color);
