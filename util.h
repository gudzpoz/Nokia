#ifndef CONSOLE_GRAPHICS_H_
#define CONSOLE_GRAPHICS_H_

#include "console.h"

void fill_char_at(int x, int y, char c);
void fill_text(int x, int y, const char* c);
void fill_line(int x1, int y1, int x2, int y2, char c);
void fill_rectangle(int x, int y, int width, int height, char c);
void draw_box(int x, int y, int width, int height, int thickness, char c);
int max(int i, int j);
int min(int i, int j);
int strcount(char* string, char c);
void strdel(char* to, char* from, int start, int length);

#endif /* CONSOLE_GRAPHICS_H_ */
