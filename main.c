#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "console.h"
#include "util.h"
#include "game.h"

#define QUIT_GAME -1
#define START_GAME 1
#define NOKIA_NAME "Nokia Game"

int welcome_loop(struct Game* game);
void start_game(struct Game* game);

int main(int argc, char* argv[]);
int WinMain() {
	return main(0, 0);
}

int main(int argc, char* argv[]) {
	init_console(NOKIA_NAME);

	struct Game* game = initiate_game(time(0));
	if(game == 0) {
		printf("Check your memory status and try again later... :(\n");
		printf("ENTER to exit... ");
		wait_getch();
		return EXIT_FAILURE;
	} else {
		int selected = welcome_loop(game);
		if(selected == START_GAME) {
			printf("Start Game!\n");
			start_game(game);
			display_result(game);
			while(wait_getch() != '\n') {
				;
			}
			free_game(game);
			quit_console();
			return EXIT_SUCCESS;
		} else if(selected == QUIT_GAME) {
			clear_screen();
			free_game(game);
			quit_console();
			return EXIT_SUCCESS;
		}
	}

	return EXIT_SUCCESS;
}

int welcome_loop(struct Game* game) {
	welcome_screen(game, 0);
	int select = 1;
	while(1) {
		welcome_screen(game, select);
		char c = wait_getch();
		if(c == 'W' || c == 'S') {
			select = (select == 1)? 2 : 1;
		} else if(c == '\n' || c == ' ') {
			if(select == 2) {
				return QUIT_GAME;
			} else if(select == 1) {
				return START_GAME;
			}
		}
	}
}

int hurry_up(int tick, int duration) {
	if(duration > 16) {
		/*
		 * Sigmoid function:
		 * tick == 0 => duration == 25 (1.5 sec);
		 * tick == 3000 (3 min) => duration == 16 (1 sec);
		 */
		return 16 + 9.0/(1.0 + exp(tick - 1500));
	} else {
		return duration;
	}
}

const char* guideText[24] = {
	"  |   1. You control the left block! **W, S**  to **move up or down**;          ",
	"##|                                  **A, D**  to **rotate**;                   ",
	"# |                                  **SPACE** to **shoot**!                    ",
	"  |                                                                             ",
	"  |                                                                             ",
	"  |                   2. There's enemy coming from right side!       $$$$$$$$   ",
	"  |                                                                    $$$$$$   ",
	"  |                                                                   $$$$$$$   ",
	"  |                                                                             ",
	"  |                                                                             ",
	"  |   3. Shoot the bullet ->       ...     And turn enemy into **RECTANGLE**!   ",
	"  |                               $$$$$$$$      $$$$$$$$                        ",
	"  |    ##    crashing into it     >>$$$$$$  --> ##$$$$$$  --->  Enemy killed.   ",
	"  |    #            -->           >$$$$$$$      #$$$$$$$                        ",
	"  |                                                                             ",
	"  |   4. You can see where your block will potentially hit by '>'s.             ",
	"  |                                                                             ",
	"##|                                                                 >>$$$$      ",
	"##|                                                              >>$$$$$$$      ",
	"  |                                                                $$$$$$$      ",
	"  |                                                                             ",
	"  |$$$$$$             5. If the enemy crosses the line, you fail.               ",
	"  |$$$$$$                                                                       ",
	"  |  $$$$                                                                       ",
};

void guide(struct Game* game) {
	pause_update();
	for(int i = 0; i != 24; ++i) {
		set_cursor(0, i);
		for(int j = 0; j != strlen(guideText[i]); ++j) {
			char c = guideText[i][j];
			if(c == '#') {
				set_color(YELLOW);
				putch(c);
			}
			else if(c == '|' || c == '$' || c == '>') {
				set_color(WHITE);
				putch(c);
			} else if(c == '*') {
				set_color(RED);
				putch(c);
			} else {
				set_color(CYAN);
				putch(c);
			}
		}
	}
	resume_update();
	wait_getch();
}

void start_game(struct Game* game) {
	do_animation(game);
	start_BGM();
	guide(game);
	initial_render(game);

	int tick = 0;
	int scrollDuration = 25;
	load_bullet(game, &X, 0);
	add_enermy(game);
	add_enermy(game);
	add_enermy(game);
	add_enermy(game);
	for(int i = 0; i != 10; ++i) {
		delay(30);
		scroll(game, 0);
	}
	while(1) {
		render(game);
		bullet_move(game);
		if(tick % scrollDuration == 0) {
			if(tick % (15 * scrollDuration) == 0) {
				add_enermy(game);
			}
			scrollDuration = hurry_up(tick, scrollDuration);
			scroll(game, 0);
		}
		hinting(game);
		display_score(game);
		// visual_update(game);
		if(game->crashed) {
			return;
		}
		char c = getch();
		if(c != EOF) {
			switch (c) {
			case '\n':
			case ' ':
				play_SE(SHOOT);
				fire_reload(game);
				break;
			case 'W':
				up(game);
				break;
			case 'S':
				down(game);
				break;
			case 'A':
				rotate_holding(game, -1);
				break;
			case 'D':
				rotate_holding(game, 1);
				break;
			default:
				break;
			}
			hinting(game);
		}
		update();
		delay(60);
		++tick;
	}
	stop_BGM();
}
