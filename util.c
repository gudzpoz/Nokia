#include "util.h"
#include <math.h>
#include <string.h>
#include "console.h"
#include <stdio.h>
#include <stdlib.h>

void fill_char_at(int x, int y, char c) {
	set_cursor(x, y);
	putch(c);
}

void fill_text(int x, int y, const char* c) {
	set_cursor(x, y);
	while(*c != '\0') {
		putch(*c);
		++c;
	}
	update();
}

/*
 * Bresenham's line algorithm
 * https://en.m.wikipedia.org/wiki/Bresenham's_line_algorithm
 */
void fill_line(int x1, int y1, int x2, int y2, char c) {
	int dx = abs(x2 - x1);
	int stepX = (x1 < x2) ? 1 : -1;
	int dy = abs(y2 - y1);
	int stepY = (y1 < y2) ? 1 : -1;
	int error = ((dx > dy) ? dx : -dy) / 2, e2;

	fill_char_at(x1, y1, c);
	while(x1 != x2 || y1 != y2) {
		fill_char_at(x1, y1, c);
		e2 = error;
		if (e2 > -dx) {
			error -= dy;
			x1 += stepX;
		}
		if (e2 < dy) {
			error += dx;
			y1 += stepY;
		}
	}
	update();
}

void fill_rectangle(int x, int y, int width, int height, char c) {
	for(int i = 0; i < height; ++i, ++y) {
		set_cursor(x, y);
		for(int j = 0; j < width; ++j) {
			putch(c);
		}
	}
	update();
}

void draw_box(int x, int y, int width, int height, int thickness, char c) {
	fill_rectangle(x, y, width, thickness, c);
	fill_rectangle(x, y, thickness, height, c);
	fill_rectangle(x + width - thickness, y, thickness, height, c);
	fill_rectangle(x, y + height - thickness, width, thickness, c);
}

void fill_circle(int x, int y, int radius) {
	for(int i = -radius + 1; i < radius; ++i) {
		double halfLength = sqrt(radius*radius - i*i);
		fill_rectangle(x - halfLength, y + i, halfLength * 2, 1, ' ');
	}
}

int max(int i, int j) {
	return (i > j)? i : j;
}
int min(int i, int j) {
	return (i < j)? i : j;
}

int strcount(char* string, char c) {
	int count = 0;
	for(int i = 0; string[i] != '\0'; ++i) {
		if(string[i] == c) {
			++count;
		}
	}
	return count;
}

void strdel(char* to, char* from, int start, int length) {
	strncpy(to, from, sizeof(char) * start);
	strcpy(&to[start], &from[start + length]);
}
