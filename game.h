#ifndef GAME_H_
#define GAME_H_

#include "console.h"

#define DEFAULT_ENEMY_CHAR '$'
#define DEFAULT_BULLET_CHAR '#'
#define COLORED_ENEMY_CHAR ' '
#define COLORED_BULLET_CHAR ' '

#define WELCOME_BOX_WIDTH 20
#define WELCOME_BOX_HEIGHT 11

#define MIN_LINE_NUMBER 3
#define MAX_LINE_NUMBER 6

#define MAX_LINE_LENGTH 11

struct Block {
	int x, y;
	char* dots;
	enum Color color;
};

const static struct Block X = {
	/*
	 * #
	 * ##
	 */
	.dots = "11\n11",
	.color = WHITE
};

const static struct Block L = {
	/*
	 * #
	 * ##
	 */
	.dots = "10\n11",
	.color = WHITE
};

const static struct Block I = {
	/*
	 * #
	 * #
	 */
	.dots = "1\n1",
	.color = WHITE
};

const static struct Block O = {
	/*
	 * #
	 */
	.dots = "1",
	.color = WHITE
};

struct BlockList {
	struct Block* block;
	struct BlockList* next;
};

struct Enemy {
	int lineNumber;
	int* lines;
	int x, y;
};

struct EnemyList {
	struct Enemy* enemy;
	struct EnemyList* next;
};

struct Game {
	int seed;
	int score;
	int crashed;
	char enemyChar, bulletChar;
	enum Color color, background;
	unsigned int drawStatus;
	struct Block* holding;
	struct BlockList* movingBlocks;
	struct EnemyList* enemies;
};

/*
 * From game.c
 */
void draw_rectangle(int x0, int y0, int boxWidth, int boxHeight, char c);
void dot(int x, int y, char c);
int lil_max(int i, int j);
int lil_min(int i, int j);
void draw_circle(int x, int y, int radius);
void free_block(struct Block* block);
void free_block_list(struct BlockList* list);
void free_enemy_list(struct EnemyList* list);
void free_block(struct Block* block);
void free_game(struct Game* game);
void draw_dots(int x, int y, char* dots, int erase, struct Game* game);
void update_block(int prev_x, int prev_y, struct Block* block, struct Game* game);
int strcount(char* string, char c);
void strdel(char* to, char* from, int start, int length);
void erase_block(int x, int y, char* dots, struct Game* game);

struct Game* initiate_game(int seed);
void welcome_screen(struct Game* game, int select);
void do_animation(struct Game* game);
void render(struct Game* game);
void initial_render(struct Game* game);
void free_game(struct Game* game);
void hinting(struct Game* game);
void display_score(struct Game* game);
void display_result(struct Game* game);
void scroll(struct Game* game, int redraw);
void bullet_move(struct Game* game);
void visual_update(struct Game* game);
void load_bullet(struct Game* game, const struct Block* bullet, const struct Block* old);
void fire_reload(struct Game* game);
void add_enermy(struct Game* game);
void rotate_holding(struct Game* game, int direction);
void up(struct Game* game);
void down(struct Game* game);
int check_dispeled(struct Game* game, struct Block* block);
void restore_color();

#endif /* GAME_H_ */
