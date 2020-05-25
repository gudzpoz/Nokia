#include "game.h"
#include "util.h"
#include "console.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>

static int* summingCache = 0;
static int width, height;

struct Game* initiate_game(int seed) {
	srand(seed);
	width = get_console_width();
	height = get_console_height();

	struct Game* game = malloc(sizeof(struct Game));
	if(game == 0) {
		return 0;
	}

	game->seed = seed;
	game->score = 0;
	game->crashed = 0;
	game->holding = 0;
	game->enemyChar = DEFAULT_ENEMY_CHAR;
	game->bulletChar = DEFAULT_BULLET_CHAR;
	game->movingBlocks = malloc(sizeof(struct BlockList));
	game->enemies = malloc(sizeof(struct BlockList));
	game->color = WHITE;
	game->background = BLACK;
	restore_color(game);

	summingCache = malloc(sizeof(int) * height);
	if(summingCache
	   && game->movingBlocks
	   && game->enemies) {
		game->movingBlocks->next = 0;
		game->enemies->next = 0;
		return game;
	} else {
		free(summingCache);
		free(game->holding);
		free(game->movingBlocks);
		free(game->enemies);
		return 0;
	}
}

void restore_color(struct Game* game) {
	set_background_color(game->background);
	set_color(game->color);
}

/*
 * To be called after "initiate_game"
 */
void welcome_screen(struct Game* game, int select) {
	const static char newGame[] = "New Game";
	if(select == 0) {
		clear_screen();
		draw_box(width/2 - WELCOME_BOX_WIDTH/2,
			 height/2 - WELCOME_BOX_HEIGHT/2,
			 WELCOME_BOX_WIDTH,
			 WELCOME_BOX_HEIGHT, 2, '#');
		set_color(CYAN);
		set_background_color(BLACK);
		fill_text(width/2 - WELCOME_BOX_WIDTH/2,
			  height/2 - WELCOME_BOX_HEIGHT/2 - 1,
			  "SPACE or ENTER to select: ");
		fill_text(width/2  - WELCOME_BOX_WIDTH/2,
			  height/2 - WELCOME_BOX_HEIGHT/2 - 2,
			  "WASD to move,  ");
		fill_text(width/2 - WELCOME_BOX_WIDTH/2 + 4,
			  height/2, newGame);
		fill_text(width/2 - WELCOME_BOX_WIDTH/2 + 4,
			  height/2 + 2, "Exit");
	} else if(select == 1) {
		set_background_color(game->background);
		fill_char_at(width / 2 - WELCOME_BOX_WIDTH/2 + 4 + sizeof(newGame), height / 2 + 2, ' ');
		set_color(game->background);
		set_background_color(game->color);
		fill_char_at(width / 2 - WELCOME_BOX_WIDTH/2 + 4 + sizeof(newGame), height / 2, '<');
	} else if(select == 2) {
		set_background_color(game->background);
		fill_char_at(width / 2 - WELCOME_BOX_WIDTH/2 + 4 + sizeof(newGame), height / 2, ' ');
		set_color(game->background);
		set_background_color(game->color);
		fill_char_at(width / 2 - WELCOME_BOX_WIDTH/2 + 4 + sizeof(newGame), height / 2 + 2, '<');
	} else if(select == -1) {
		clear_screen();
	}
	restore_color(game);
	update();
}

void do_animation(struct Game* game) {
	set_color(CYAN);
	for(int i = WELCOME_BOX_WIDTH/2 - 1, j = WELCOME_BOX_HEIGHT/2 - 1;
	    i <= width / 2 || j <= height / 2; ++i, ++j) {
		delay(30);
		draw_box(width / 2 - i, height / 2 - j, 2*i, 2*j, 1, '#');
	}
	clear_screen();
}

void free_block_list(struct BlockList* list) {
	do {
		struct BlockList* next = list->next;
		if(list->block != 0) {
			free(list->block->dots);
			free(list->block);
		}
		free(list);
		list = next;
	} while(list != 0);
}

void free_enemy_list(struct EnemyList* list) {
	while(list != 0) {
		struct EnemyList* next = list->next;
		if(list->enemy != 0) {
			free(list->enemy->lines);
			free(list->enemy);
		}
		free(list);
		list = next;
	}
}

void free_game(struct Game* game) {
	free(game->holding->dots);
	free(game->holding);
	free_enemy_list(game->enemies);
	free_block_list(game->movingBlocks);
	free(game);
}

void hinting(struct Game* game) {
	pause_update();
	memset(summingCache, 0, height * sizeof(int));
	struct EnemyList* enemyList = game->enemies;
	while(enemyList->next != 0) {
		enemyList = enemyList->next;
		struct Enemy* enemy = enemyList->enemy;
		for(int i = 0; i != enemy->lineNumber; ++i) {
			if(summingCache[i + enemy->y] == 0) {
				summingCache[i + enemy->y] = enemy->x - enemy->lines[i];
			} else {
				if(summingCache[i + enemy->y] > (enemy->x - enemy->lines[i])) {
					summingCache[i + enemy->y] = enemy->x - enemy->lines[i];
				}
			}
		}
	}

	static char* dots = 0;
	static int blockHeight = 0;
	static int y0;
	static int x0[3];
	static int pixels[3];
	if(dots == 0) {
		dots = game->holding->dots;
	} else {
		for(int i = 0; i != blockHeight; ++i) {
			fill_rectangle(x0[i] - pixels[i] + 1, y0 + i, pixels[i], 1, ' ');
		}
		dots = game->holding->dots;
	}
	blockHeight = strcount(dots, '\n') + 1;
	y0 = game->holding->y;
	int y = y0;
	int i = -1;
	int pixelPerLine = 0;
	do {
		++i;
		if(dots[i] == '1') {
			++pixelPerLine;
		} else if(dots[i] == '\n' || dots[i] == '\0') {
			if(summingCache[y] == 0) {
				pixels[y - y0] = pixelPerLine;
				x0[y - y0] = width - 1;
				fill_rectangle(width - pixelPerLine, y, pixelPerLine, 1, '>');
			} else {
				pixels[y - y0] = pixelPerLine;
				x0[y - y0] = min(width - 1, summingCache[y]);
				fill_rectangle(x0[y - y0] - pixels[y - y0] + 1, y, pixelPerLine, 1, '>');
			}
			pixelPerLine = 0;
			++y;
		}
	} while(dots[i] != '\0');
	scroll(game, 1);
	resume_update();
}

void display_score(struct Game* game) {
	static char string[32];
	if(game->score <= 99999) {
		sprintf(string, "Score: %05d ", game->score);
	} else {
		sprintf(string, "Score: %05d+", 99999);
	}
	fill_text(4, height - 1, string);
}

void display_result(struct Game* game) {
	static char score[16];
	const char thanks[] = "THANKS FOR PLAYING!";
	const int RESULT_BOX_WIDTH = sizeof(thanks)/sizeof(char) + 10;
	const int RESULT_BOX_HEIGHT = 16;
	set_color(CYAN);
	set_background_color(BLACK);
	fill_rectangle(width/2 - RESULT_BOX_WIDTH/2,
		       height/2 - RESULT_BOX_HEIGHT/2,
		       RESULT_BOX_WIDTH,
		       RESULT_BOX_HEIGHT, ' ');
	draw_box(width/2 - RESULT_BOX_WIDTH/2,
		 height/2 - RESULT_BOX_HEIGHT/2,
		 RESULT_BOX_WIDTH,
		 RESULT_BOX_HEIGHT, 2, '#');
	fill_text(width/2 - RESULT_BOX_WIDTH/2 + 5,
		  height/2 - 2, thanks);
	fill_text(width/2 - RESULT_BOX_WIDTH/2 + 5,
		  height/2 + 0, "Score: ");
	set_color(WHITE);
	sprintf(score, "%d", game->score);
	fill_text(width/2 - RESULT_BOX_WIDTH/2 + 12,
		  height/2 + 1, score);
	set_color(YELLOW);
	fill_text(width/2 - RESULT_BOX_WIDTH/2,
		  height/2 + RESULT_BOX_HEIGHT/2 + 1, "ENTER to exit. ");
	set_color(CYAN);
	fill_text(4, height - 1, "BGM, SE: Maoudamashii.      A game inspired by a Nokia game.");
}

void add_enermy(struct Game* game) {
	struct EnemyList* list = malloc(sizeof(struct EnemyList));
	list->next = 0;
	struct Enemy* newEnemy = malloc(sizeof(struct Enemy));
	list->enemy = newEnemy;
	int lineNumber = rand() % (MAX_LINE_NUMBER - MIN_LINE_NUMBER + 1) + MIN_LINE_NUMBER;
	newEnemy->lineNumber = lineNumber;
	newEnemy->lines = malloc(sizeof(int) * lineNumber);
	for(int i = 0; i != lineNumber; ++i) {
		newEnemy->lines[i] = rand() % MAX_LINE_LENGTH + 1;
	}
	int allEqual = 1;
	for(int i = 0; i != lineNumber - 1; ++i) {
		if(newEnemy->lines[i] != newEnemy->lines[i+1]) {
			allEqual = 0;
			break;
		}
	}
	if(allEqual == 1) {
		++newEnemy->lines[rand() % lineNumber];
	}

	memset(summingCache, 0, height * sizeof(int));
	struct EnemyList* enemyList = game->enemies;
	while(enemyList->next != 0) {
		enemyList = enemyList->next;
		struct Enemy* enemy = enemyList->enemy;
		for(int i = 0; i != enemy->lineNumber; ++i) {
			// TODO: decide whether to remove the temporary "extent"
			int extent = summingCache[enemy->y + i - 1];
			extent = max(extent, enemy->x);
			summingCache[enemy->y + i - 1] = extent;
		}
	}
	int min = INT_MAX, line = 0;
	int dy =  2 + rand() % 4;
	for(int i = 1; i < height - newEnemy->lineNumber - dy - 2; ++i) {
		int max = 0;
		for(int j = 0; j < newEnemy->lineNumber + 2 + dy; ++j) {
			if(summingCache[i + j] > max) {
				max = summingCache[i + j];
			}
		}
		if(max < min) {
			min = max;
			line = i + 1;
		}
	}
	int boxWidth = 0;
	for(int i = 0; i != newEnemy->lineNumber; ++i) {
		if(boxWidth < newEnemy->lines[i]) {
			boxWidth = newEnemy->lines[i];
		}
	}
	newEnemy->x = max(min, width) + boxWidth + 2 + (rand()%5) + 1;
	newEnemy->y = line + (rand()%dy) + 1;
	enemyList->next = list;
}

void update_enemy(struct Game* game, struct Enemy* enemy) {
	set_color(game->color);
	fill_rectangle(enemy->x + 1, enemy->y, 1, enemy->lineNumber, ' ');
	for(int i = 0; i != enemy->lineNumber; ++i) {
		fill_rectangle(enemy->x - enemy->lines[i] + 1, enemy->y + i, enemy->lines[i], 1, game->enemyChar);
		if(enemy->x - enemy->lines[i] + 1 <= 3) {
			game->crashed = 1;
		}
	}
}

void scroll(struct Game* game, int redraw) {
	struct EnemyList* list = game->enemies->next;
	while(list != 0) {
		if(redraw == 0) {
			--list->enemy->x;
		}
		update_enemy(game, list->enemy);
		list = list->next;
	}
}

void draw_dots(int x, int y, char* dots, int erase, struct Game* game) {
	int x0 = x;
	for(int i = 0; dots[i] != '\0'; ++i) {
		char c = dots[i];
		if(c == '\n') {
			x0 = x;
			++y;
		} else {
			if(c != '0') {
				set_color(YELLOW);
				if(erase == 1) {
					fill_char_at(x0, y, ' ');
				} else if(c == '1') {
					fill_char_at(x0, y, game->bulletChar);
				} else {
					fill_char_at(x0, y, c);
				}
				restore_color(game);
			}
			++x0;
		}
	}
}

void update_block(int prev_x, int prev_y, struct Block* block, struct Game* game) {
	restore_color(game);
	draw_dots(prev_x, prev_y, block->dots, 1, game);
	draw_dots(block->x, block->y, block->dots, 0, game);
	update();
}

void render(struct Game* game) {
	restore_color(game);
	if(game->holding != 0) {
		pause_update();
		fill_rectangle(game->holding->x, game->holding->y - 1, 2, 4, ' ');
		fill_rectangle(2, game->holding->y - 1, 1, 4, '|');
		update_block(game->holding->x, game->holding->y, game->holding, game);
		resume_update();
	}
}

void initial_render(struct Game* game) {
	clear_screen();
	restore_color(game);
	fill_rectangle(2, 0, 1, height, '|');
}

int check_dispeled(struct Game* game, struct Block* block) {
	struct EnemyList* parent = game->enemies;
	struct EnemyList* list = parent->next;
	while(list != 0) {
		struct Enemy* enemy = list->enemy;
		char* dots = block->dots;

		// duplicate of "draw_dots"
		int x0 = block->x;
		int y0 = block->y;
		int pixelPerLine = 0;
		int updated = 0;
		for(int i = 0; dots[i] != '\0'; ++i) {
			char c = dots[i];
			if(c == '\n') {
				x0 = block->x;
				pixelPerLine = 0;
				++y0;
			} else {
				if(c != '0') {
					++pixelPerLine;
					if(enemy->y <= y0 && y0 <= (enemy->y + enemy->lineNumber - 1)) {
						if((enemy->x - enemy->lines[y0 - enemy->y]) <= x0) {
							enemy->lines[y0 - enemy->y] += pixelPerLine;

							draw_dots(block->x, block->y, block->dots, 1, game);
							int length = strlen(dots);
							char* newDots = malloc(sizeof(char) * length - pixelPerLine);
							strdel(newDots, dots, i - pixelPerLine + 1, pixelPerLine);
							free(block->dots);
							block->dots = newDots;
							i -= pixelPerLine;
							dots = block->dots;
							draw_dots(block->x, block->y, block->dots, 0, game);

							pixelPerLine = 0;
							updated = 1;
							play_SE(HIT);
						}
					}
					++x0;
				} else {
					pixelPerLine = 0;
					++x0;
				}
			}
		}
		if(updated == 1) {
			update_enemy(game, enemy);
		}
		int dispeled = 1;
		for(int i = 0; i != enemy->lineNumber - 1; ++i) {
			if(enemy->lines[i] != enemy->lines[i + 1]) {
				dispeled = 0;
			}
		}
		if(dispeled == 1) {
			play_SE(SCORE);
			// TODO
			set_color(game->color);
			game->score += enemy->lines[0] * enemy->lineNumber * 5;
			fill_rectangle(enemy->x - enemy->lines[0] + 1,
				       enemy->y, enemy->lines[0], enemy->lineNumber, ' ');
			parent->next = list->next;
			free(list->enemy->lines);
			free(list->enemy);
			free(list);
		} else {
			parent = list;
		}
		list = parent->next;
	}
	return 0;
}

void bullet_move(struct Game* game) {
	struct BlockList* parent = game->movingBlocks;
	struct BlockList* block = parent->next;
	while(block != 0) {
		if(block->block->x > width) {
			game->score -= strcount(block->block->dots, '1') * 30;
			parent->next = block->next;
			free(block->block);
			free(block);
			block = parent->next;
		} else {
			int prev_x = block->block->x;
			int prev_y = block->block->y;
			block->block->x += 1;
			update_block(prev_x, prev_y, block->block, game);
			check_dispeled(game, block->block);
			parent = block;
			block = parent->next;
		}
	}
}
void visual_update(struct Game* game) {
	render(game);
}

void load_bullet(struct Game* game, const struct Block* bullet, const struct Block* old) {
	if(game->holding == 0) {
		struct Block* block = malloc(sizeof(struct Block));
		block->dots = malloc(strlen(bullet->dots) * sizeof(char));
		strcpy(block->dots, bullet->dots);
		block->color = bullet->color;
		if(old == 0) {
			block->x = 0;
			block->y = height / 2;
		} else {
			block->x = old->x;
			block->y = old->y;
		}
		game->holding = block;
		render(game);
	} else {
		return;
	}
}

void fire_reload(struct Game* game) {
	struct BlockList* list = game->movingBlocks;
	while(list->next != 0) {
		list = list->next;
	}
	list->next = malloc(sizeof(struct BlockList));
	list->next->next = 0;
	list->next->block = game->holding;
	game->holding = 0;
	switch (rand() % 4) {
	case 0:
		load_bullet(game, &O, list->next->block);
		break;
	case 1:
		load_bullet(game, &I, list->next->block);
		break;
	case 2:
		load_bullet(game, &L, list->next->block);
		break;
	case 3:
		load_bullet(game, &X, list->next->block);
		break;
	}
}

void rotate_holding(struct Game* game, int direction) {
	char* dots;
	if(direction > 0) {
		for(; direction != 0; --direction) {
			switch (strlen(game->holding->dots)) {
			case 1: // #
				break;
			case 2: // ##
				free(game->holding->dots);
				game->holding->dots = malloc(4 * sizeof(char));
				strcpy(game->holding->dots, "1\n1");
				break;
			case 3: // #\n#
				free(game->holding->dots);
				game->holding->dots = malloc(3 * sizeof(char));
				strcpy(game->holding->dots, "11");
				break;
			case 5: // xx\nxx
				dots = game->holding->dots;
				char c = dots[0];
				dots[0] = dots[1];
				dots[1] = dots[4];
				dots[4] = dots[3];
				dots[3] = c;
				break;
			default:
				break;
			}
		}
	} else {
		for(; direction != 0; ++direction) {
			switch (strlen(game->holding->dots)) {
			case 1: // #
				break;
			case 2: // ##
				free(game->holding->dots);
				game->holding->dots = malloc(4 * sizeof(char));
				strcpy(game->holding->dots, "1\n1");
				break;
			case 3: // #\n#
				free(game->holding->dots);
				game->holding->dots = malloc(3 * sizeof(char));
				strcpy(game->holding->dots, "11");
				break;
			case 5: // xx\nxx
				dots = game->holding->dots;
				char c = dots[0];
				dots[0] = dots[3];
				dots[3] = dots[4];
				dots[4] = dots[1];
				dots[1] = c;
				break;
			default:
				break;
			}
		}
	}
}

void up(struct Game* game) {
	if(game->holding->y > 0) {
		--game->holding->y;
	}
	render(game);
}

void down(struct Game* game) {
	if(game->holding->y < height - 2) {
		++game->holding->y;
	}
	render(game);
}
