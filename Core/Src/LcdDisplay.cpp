/*
 * LcdDisplay.cpp
 *
 *  Created on: Aug 25, 2023
 *      Author: Mikołaj
 */

#include "spi.h"
#include "stdlib.h"
#include <cmath>

#define ST7735S_SLPOUT			0x11
#define ST7735S_DISPOFF			0x28
#define ST7735S_DISPON			0x29
#define ST7735S_CASET			0x2a
#define ST7735S_RASET			0x2b
#define ST7735S_RAMWR			0x2c
#define ST7735S_MADCTL			0x36
#define ST7735S_COLMOD			0x3a
#define ST7735S_FRMCTR1			0xb1
#define ST7735S_FRMCTR2			0xb2
#define ST7735S_FRMCTR3			0xb3
#define ST7735S_INVCTR			0xb4
#define ST7735S_PWCTR1			0xc0
#define ST7735S_PWCTR2			0xc1
#define ST7735S_PWCTR3			0xc2
#define ST7735S_PWCTR4			0xc3
#define ST7735S_PWCTR5			0xc4
#define ST7735S_VMCTR1			0xc5
#define ST7735S_GAMCTRP1		0xe0
#define ST7735S_GAMCTRN1		0xe1

#define CMD(x)			((x) | 0x100)
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define LCD_OFFSET_X  1
#define LCD_OFFSET_Y  2

#include <LcdDisplay.h>

bool zero[] = {0, 1, 1, 1, 0,
		1, 1, 0, 1, 1,
		1, 0, 0, 0, 1,
		1, 0, 0, 0, 1,
		1, 0, 0, 0, 1,
		1, 0, 0, 0, 1,
		1, 1, 0, 1, 1,
		0, 1, 1, 1, 0};

struct RGB{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

class Char{
private:
	uint32_t rows;
};

RGB convertHexToRgb(uint16_t hexValue){
	struct RGB rgbColor;
	  rgbColor.r = ((hexValue >> 16) & 0xFF) / 255.0;
	  rgbColor.g = ((hexValue >> 8) & 0xFF) / 255.0;
	  rgbColor.b = ((hexValue) & 0xFF) / 255.0;
	  return rgbColor;
}

static uint16_t frameBuffer[LcdDisplay::LCD_WIDTH * LcdDisplay::LCD_HEIGHT]; //can be inside the class but it's here to show memory usage more clearly

LcdDisplay::LcdDisplay() {
	const uint16_t initTable[]  {
	  CMD(ST7735S_FRMCTR1), 0x01, 0x2c, 0x2d,
	  CMD(ST7735S_FRMCTR2), 0x01, 0x2c, 0x2d,
	  CMD(ST7735S_FRMCTR3), 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,
	  CMD(ST7735S_INVCTR), 0x07,
	  CMD(ST7735S_PWCTR1), 0xa2, 0x02, 0x84,
	  CMD(ST7735S_PWCTR2), 0xc5,
	  CMD(ST7735S_PWCTR3), 0x0a, 0x00,
	  CMD(ST7735S_PWCTR4), 0x8a, 0x2a,
	  CMD(ST7735S_PWCTR5), 0x8a, 0xee,
	  CMD(ST7735S_VMCTR1), 0x0e,
	  CMD(ST7735S_GAMCTRP1), 0x0f, 0x1a, 0x0f, 0x18, 0x2f, 0x28, 0x20, 0x22,
	                         0x1f, 0x1b, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10,
	  CMD(ST7735S_GAMCTRN1), 0x0f, 0x1b, 0x0f, 0x17, 0x33, 0x2c, 0x29, 0x2e,
	                         0x30, 0x30, 0x39, 0x3f, 0x00, 0x07, 0x03, 0x10,
	  CMD(0xf0), 0x01,
	  CMD(0xf6), 0x00,
	  CMD(ST7735S_COLMOD), 0x05,
	  CMD(ST7735S_MADCTL), 0xa0,
	};

	  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
	  HAL_Delay(100);
	  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	  HAL_Delay(100);

	  for (auto hexMsg: initTable) {
		  lcdSend(hexMsg);
	  }

	  HAL_Delay(200);

	  lcdCmd(ST7735S_SLPOUT);
	  HAL_Delay(120);

	  lcdCmd(ST7735S_DISPON);

	  lcdCreateRectangle(0, 0, 160, 128, BLACK);

}

 void LcdDisplay::lcdCmd(uint8_t cmd)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

 void LcdDisplay::lcdSend(uint16_t value)
 {
 	if (value & 0x100) {
 		lcdCmd(value);
 	} else {
 		lcdData(value);
 	}
 }

 void LcdDisplay::lcdData(uint8_t data)
 {
 	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
 	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
 	HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
 	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
 }

void LcdDisplay::lcdCreateRectangle(int x, int y, int width, int height, uint16_t color){
 	lcdSetWindow(x, y, width, height);

 	lcdCmd(ST7735S_RAMWR);
 	for (int i = 0; i < width * height; i++)
 		lcdDataTwoBajts(color);
 }

void LcdDisplay::lcdDataTwoBajts(uint16_t value)
 {
 	lcdData(value >> 8);
 	lcdData(value);
 }

void LcdDisplay::lcdSetWindow(int x, int y, int width, int height)
 {
 	lcdCmd(ST7735S_CASET);
 	lcdDataTwoBajts(LCD_OFFSET_X + x);
 	lcdDataTwoBajts(LCD_OFFSET_X + x + width - 1);

 	lcdCmd(ST7735S_RASET);
 	lcdDataTwoBajts(LCD_OFFSET_Y + y);
 	lcdDataTwoBajts(LCD_OFFSET_Y + y + height- 1);
 }

void LcdDisplay::putPixel(int x, int y, uint16_t color)
{
	if(x <= LcdDisplay::LCD_WIDTH && y <= LcdDisplay::LCD_HEIGHT && x >= 0 && y >= 0){
		frameBuffer[x + y * LCD_WIDTH] = color;
	}
}

void LcdDisplay::show(){
	lcdSetWindow(0, 0, LCD_WIDTH, LCD_HEIGHT);

	lcdCmd(ST7735S_RAMWR);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, (uint8_t*)frameBuffer, sizeof(frameBuffer), HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void
LcdDisplay::drawCircle(int8_t xc, int8_t yc, int8_t r, uint16_t color)
{
	int8_t x = 0;
	int8_t y = r;
	int8_t d = 4 - 2 * r;

    putPixel(xc + x, yc + y, color);
    putPixel(xc + x, yc - y, color);
    putPixel(xc + y, yc + x, color);
    putPixel(xc - y, yc + x, color);

    while (y >= x) {
        if (d > 0) {
            d = d + 4 * (x - y) + 10;
            y--;
            x++;
        } else {
            d = d + 4 * x + 6;
            x++;
        }

        putPixel(xc + x, yc + y, color);
        putPixel(xc - x, yc + y, color);
        putPixel(xc + x, yc - y, color);
        putPixel(xc - x, yc - y, color);
        putPixel(xc + y, yc + x, color);
        putPixel(xc - y, yc + x, color);
        putPixel(xc + y, yc - x, color);
        putPixel(xc - y, yc - x, color);
    }
}

void LcdDisplay::drawRectangle(int8_t xp, int8_t yp, int8_t width, int8_t height, uint16_t color){
	for (int y = 0; y < height; y++) {
	    for (int x = 0; x < width; x++) {
	    	putPixel(xp+x, yp+y, color);
	    }
	  }
}

void LcdDisplay::fadedRectangle(int8_t xp, int8_t yp, int8_t width, int8_t height, uint16_t color , uint16_t endColor){
	uint16_t difference = color-endColor;
	convertHexToRgb(color);
	for (int y = 0; y < height; y++) {
	    for (int x = 0; x < width; x++) {
	    	putPixel(xp+x, yp+y, color);
	    }
	    color -= difference / (height*10.0);
	  }
}

void LcdDisplay::drawFilledCircle(int8_t xc, int8_t yc, int8_t r, uint16_t color){
	int16_t x = 0;
	int16_t y = r;
	int16_t d = - 2 * r;
	drawLine(xc - r, yc, xc + r, yc, color);
	while (y >= x) {
		drawLine(xc - x, yc + y, xc + x, yc + y, color);
		drawLine(xc - x, yc - y, xc + x, yc - y, color);
		drawLine(xc - y, yc + x, xc + y, yc + x, color);
		drawLine(xc - y, yc - x, xc + y, yc - x, color);

		if (d <= 0) {
			d = d + 4 * x + 6;
			x++;
		} else {
			d = d + 4 * (x - y) + 10;
			x++;
			y--;
		}
	}
}

void LcdDisplay::drawLine(float xb, float yb, float xe, float ye, uint16_t color){
	auto a = (ye - yb)/(xe - xb);
	auto b = yb - a*xb;
	int8_t end = 0;
	int8_t start = 0;
	if (xb > xe){
		end = xb;
		start = xe;
	}else{
		end = xe;
		start = xb;
	}
	while(start <= end){
		putPixel(start, a*start + b, color);
		start++;
	}

}

void LcdDisplay::drawTriangle(int8_t x0, int8_t y0, int8_t x1, int8_t y1, int8_t x2, int8_t y2, int16_t color){
	drawLine(x0, y0, x1, y1, color);
	drawLine(x0, y0, x2, y2, color);
	drawLine(x1, y1, x2, y2, color);
	auto a10 = (y1 - y0)/((x1 - x0)*1.0);
	auto b10 = y1 - a10*x1;
	auto a12 = (y1 - y2)/((x1 - x2)*1.0);
	auto b12 = (y1 - a12*x1);
	auto a20 = (y2 - y0)/((x2 - x0)*1.0);
	auto b20 = (y2 - a20*x2);
	if(y0 <= y2 && y0 >= y1){
		drawBottom(y2, y0, x0, a12, b12, a20, b20, color);
		drawTop(y1, y0, x0, a12, b12, a10, b10, color);
	}
	else if(y0 <= y1 && y0 >= y2){
		drawBottom(y1, y0, x0, a12, b12, a10, b10, color);
		drawTop(y2, y0, x0, a12, b12, a20, b20, color);
	}
	else if(y1 <= y2 && y1 >= y0){
		drawBottom(y2, y1, x1, a12, b12, a20, b20, color);
		drawTop(y0, y1, x1, a20, b20, a10, b10, color);
	}
	else if (y1 <= y0 && y1 >= y2){
		drawBottom(y0, y1, x1, a10, b10, a20, b20, color);
		drawTop(y2, y1, x1, a20, b20, a12, b12, color);
	}
	else if(y2 <= y0 && y2 >= y1){
		drawBottom(y0, y2, x2, a10, b10, a20, b20, color);
		drawTop(y1, y2, x2, a10, b10, a12, b12, color);
	}
	else{
		drawBottom(y1, y2, x2, a10, b10, a12, b12, color);
		drawTop(y0, y2, x2, a10, b10, a20, b20, color);
	}
}

void LcdDisplay::drawBottom(int8_t bottomY, int8_t currentY, int8_t currentX, float firstA, float firstB, float secondA, float secondB, int16_t color){
	int firstX = 0;
	int secondX = 0;
	while(currentY < bottomY){
		if(isinff(firstA)){
			firstX = currentX;
		}
		else{
			firstX = (currentY - firstB)/firstA;
		}
		if(isinff(secondB)){
			secondX = currentX;
		}
		else{
			secondX = (currentY - secondB)/secondA;
		}
		drawHorizontalLine(firstX, currentY, secondX, currentY, color);
		currentY ++;
	}
}

void LcdDisplay::drawTop(int8_t topY, int8_t currentY, int8_t currentX, float firstA, float firstB, float secondA, float secondB, int16_t color){
	int firstX = 0;
	int secondX = 0;
	while(currentY > topY){
		if(isinff(firstA)){
			firstX = currentX;
		}
		else{
			firstX = (currentY - firstB)/firstA;
		}
		if(isinff(secondB)){
			secondX = currentX;
		}
		else{
			secondX = (currentY - secondB)/secondA;
		}
		drawHorizontalLine(firstX, currentY, secondX, currentY, color);
		currentY --;
	}
}

void LcdDisplay::drawHorizontalLine(int8_t xb, int8_t yb, int8_t xe, int8_t ye, uint16_t color){
	int8_t end = 0;
	int8_t start = 0;
	if (xb > xe){
		end = xb;
		start = xe;
	}else{
		end = xe;
		start = xb;
	}
	while(start <= end){
		putPixel(start, ye, color);
		start++;
	}
}
void LcdDisplay::drawNumber(int8_t x, int8_t y, uint16_t color){
//	for(int i = 0; i < 5*8; i++){
//		if(zero[i]){
//			putPixel(x + i%5, y+ int(i/5), color);
//		}
//	}
	for(int i = 0; i < 8; i++){
		printRow(i, x, y+i, color);
	}
}

void LcdDisplay::printRow(uint8_t ronType, int8_t x, int8_t y, uint16_t color){
	bool zeros[8][5] = {{0, 1, 1, 1, 0},
			{1, 1, 0, 1, 1},
			{1, 0, 0, 0, 1},
			{1, 0, 0, 0, 1},
			{1, 0, 0, 0, 1},
			{1, 0, 0, 0, 1},
			{1, 1, 0, 1, 1},
			{0, 1, 1, 1, 0}};
	for(auto pixel : zeros[ronType]){
		if(pixel)
			putPixel(x, y, color);
		x++;
	}
}

LcdDisplay::~LcdDisplay() {
	// TODO Auto-generated destructor stub
}

