/*
 * LcdDisplay.h
 *
 *  Created on: Aug 25, 2023
 *      Author: Mikołaj
 */

#ifndef INC_LCDDISPLAY_H_
#define INC_LCDDISPLAY_H_

#include <stdint.h>

#define BLACK			0x0000
#define RED			0x00f8
#define GREEN			0xe007
#define BLUE			0x1f00
#define YELLOW			0xe0ff
#define MAGENTA			0x1ff8
#define CYAN			0xff07
#define WHITE			0xffff


class LcdDisplay {
// some part are taken form hagl library
public:
	static const auto LCD_WIDTH {80};
	static const auto LCD_HEIGHT {80};
	LcdDisplay();
	virtual ~LcdDisplay();
	void lcdCreateRectangle(int x, int y, int width, int height, uint16_t color);
	void show();
	void putPixel(int x, int y, uint16_t color);
	void drawCircle(int8_t xc, int8_t yc, int8_t r, uint16_t color);
	void drawRectangle(int8_t xp, int8_t yp, int8_t width, int8_t height, uint16_t color);
	void fadedRectangle(int8_t xp, int8_t yp, int8_t width, int8_t height, uint16_t color, uint16_t endColor);
	void drawFilledCircle(int8_t xc, int8_t yc, int8_t r, uint16_t color);
	void drawLine(float xb, float yb, float xe, float ye, uint16_t color);
	void drawTriangle(int8_t x0, int8_t y0, int8_t x1, int8_t y1, int8_t x2, int8_t y2, int16_t color);
	void drawNumber(int8_t x, int8_t y, uint16_t color);
private:
	void lcdCmd(uint8_t cmd);
	void lcdSend(uint16_t value);
	void lcdData(uint8_t data);
	void lcdSetWindow(int x, int y, int width, int height);
	void lcdDataTwoBajts(uint16_t value);
	void drawBottom(int8_t bottomY, int8_t currentY, int8_t currentX, float firstA, float firstB, float secondA, float secondB, int16_t color);
	void drawTop(int8_t topY, int8_t currentY, int8_t currentX, float firstA, float firstB, float secondA, float secondB, int16_t color);
	void drawHorizontalLine(int8_t xb, int8_t yb, int8_t xe, int8_t ye, uint16_t color);
	void printRow(uint8_t ronType, int8_t x, int8_t y, uint16_t color);
};

#endif /* INC_LCDDISPLAY_H_ */
