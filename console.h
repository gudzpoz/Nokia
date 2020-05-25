#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <SDL2/SDL.h>

enum Color { BLACK = 0, WHITE, RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA };
enum SoundEffect { SHOOT, HIT, SCORE };
void init_console(char* title);
void quit_console();
void set_cursor(int x, int y);
void fill_char(char c);
void putch(char c);
int getch();
void delay(unsigned int mills);
void set_color(enum Color color);
void set_background_color(enum Color background);
void clear_screen();
void update();
void pause_update();
void resume_update();
int get_console_width();
int get_console_height();
int wait_getch();
void start_BGM();
void play_SE(enum SoundEffect se);
void stop_BGM();

#endif /* CONSOLE_H_ */