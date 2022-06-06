#include "pieces.h"
#include "primlib.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define WIDTH_IN_BLOCKS 10
#define HIGHT_IN_BLOCKS 30

#define FALL_DELAY 0.25

#define LINES_OF_DEATH_FROM_CELING 3

#define END_TEXT_LINE_1 "congratulations!!!"
#define END_TEXT_LINE_2 "You have LOST!!!"
// screen 1200x700

int block_array_coordinates[WIDTH_IN_BLOCKS][HIGHT_IN_BLOCKS][2] = {0};
char block_array[WIDTH_IN_BLOCKS][HIGHT_IN_BLOCKS] = {0};

int block_array_coordinates_for_next_figure[4][4][2] = {0};

enum figures {
	square,
	I,
	L,
	L_mirrored,
	N,
	N_mirrored,
	T
} current_figure,
	next_figure;
enum block_piece { empty_piece, fixed_piece, moving_piece, center_piece };
enum rotation { left, up, right, down } current_rotation, future_rotation;

int block_size, current_x, current_y;
clock_t start_time;

void scale_size_of_structures() {
	block_size = gfx_screenHeight() / HIGHT_IN_BLOCKS;
}

void initiate_block_array() {
	int center_of_screen_with_respect_to_block;
	if (WIDTH_IN_BLOCKS % 2 == 1) {
		center_of_screen_with_respect_to_block =
			gfx_screenWidth() / 2 - block_size / 2;
	} else {
		center_of_screen_with_respect_to_block =
			gfx_screenWidth() / 2 - block_size;
	}
	for (int n = 0; n < WIDTH_IN_BLOCKS; n++) {
		for (int i = 0; i < HIGHT_IN_BLOCKS; i++) {
			block_array_coordinates[n][i][0] =
				center_of_screen_with_respect_to_block +
				block_size * (n - WIDTH_IN_BLOCKS / 2);
			block_array_coordinates[n][i][1] =
				gfx_screenHeight() - 1 - block_size * i;
		}
	}
}

void initiate_next_figure_array_coordinates() {
	int starting_x = gfx_screenWidth() - 6 * block_size,
		starting_y = 6 * block_size;
	for (int n = 0; n < 4; n++) {
		for (int i = 0; i < 4; i++) {
			block_array_coordinates_for_next_figure[n][i][0] =
				starting_x + block_size * n;
			block_array_coordinates_for_next_figure[n][i][1] =
				starting_y - block_size * i;
		}
	}
}

void draw_next_figure() {
	enum block_color {
		fixed = RED,
		moving = GREEN,
		center = YELLOW
	} color_state;
	for (int n = 0; n < 4; n++) {
		for (int i = 0; i < 4; i++) {
			if (pieces[next_figure][left][n][i] != 0) {
				switch (pieces[next_figure][left][n][i]) {
				case 1:
					color_state = moving;
					break;
				case 2:
					color_state = center;
					break;
				}
				gfx_filledRect(
					block_array_coordinates_for_next_figure[n][3 - i][0],
					block_array_coordinates_for_next_figure[n][3 - i][1],
					block_array_coordinates_for_next_figure[n][3 - i][0] +
						block_size,
					block_array_coordinates_for_next_figure[n][3 - i][1] -
						block_size,
					color_state);
			}
		}
	}
}

void draw_blocks() {
	enum block_color {
		fixed = RED,
		moving = GREEN,
		center = YELLOW
	} color_state;
	for (int n = 0; n < WIDTH_IN_BLOCKS; n++) {
		for (int i = 0; i < HIGHT_IN_BLOCKS; i++) {
			if (block_array[n][i] != 0) {
				switch (block_array[n][i]) {
				case fixed_piece:
					color_state = fixed;
					break;
				case moving_piece:
					color_state = moving;
					break;
				case center_piece:
					color_state = center;
					break;
				}
				gfx_filledRect(block_array_coordinates[n][i][0],
							   block_array_coordinates[n][i][1],
							   block_array_coordinates[n][i][0] + block_size,
							   block_array_coordinates[n][i][1] - block_size,
							   color_state);
			}
		}
	}
}

int find_shift_x_of_center_square(enum rotation direction,
								  enum figures figure) {
	int shift_x;
	for (int n = 0; n < 4; n++) {
		for (int i = 0; i < 4; i++) {
			if (pieces[figure][direction][i][n] == 2) {
				shift_x = n;
				n = 4;
				break;
			}
		}
	}
	return shift_x;
}

int find_shift_y_of_center_square(enum rotation direction,
								  enum figures figure) {
	int shift_y;
	for (int n = 0; n < 4; n++) {
		for (int i = 0; i < 4; i++) {
			if (pieces[figure][direction][i][n] == 2) {
				shift_y = i;
				n = 4;
				break;
			}
		}
	}
	return shift_y;
}

void draw_border_lines() {
	gfx_line(block_array_coordinates[0][0][0] - 1, gfx_screenHeight() - 1,
			 block_array_coordinates[0][0][0] - 1, 0, BLUE);
	gfx_line(
		block_array_coordinates[WIDTH_IN_BLOCKS - 1][0][0] + 1 + block_size,
		gfx_screenHeight() - 1,
		block_array_coordinates[WIDTH_IN_BLOCKS - 1][0][0] + 1 + block_size, 0,
		BLUE);
}

void draw_all() {
	gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLACK);
	draw_border_lines();
	draw_next_figure();
	draw_blocks();
	gfx_updateScreen();
}

void draw_figure(int block_x, int block_y, enum rotation direction,
				 enum figures figure, enum block_piece piece) {
	for (int n = 0; n < 4; n++) {
		for (int i = 0; i < 4; i++) {
			if (pieces[figure][direction][i][n] != 0) {
				if (piece == fixed_piece) {
					block_array[block_x + n][block_y - i] = fixed_piece;
				} else if (piece == moving_piece &&
						   pieces[figure][direction][i][n] == 2) {
					block_array[block_x + n][block_y - i] = center_piece;
				} else {
					block_array[block_x + n][block_y - i] = moving_piece;
				}
			}
		}
	}
}

void clean_figure(int block_x, int block_y, enum rotation direction,
				  enum figures figure) {
	for (int n = 0; n < 4; n++) {
		for (int i = 0; i < 4; i++) {
			if (pieces[figure][direction][i][n] != 0) {
				block_array[block_x + n][block_y - i] = 0;
			}
		}
	}
}

bool is_move_possible(int from_block_x, int from_block_y, int to_block_x,
					 int to_block_y, enum rotation from_direction,
					 enum rotation to_direction, enum figures figure) {
	clean_figure(from_block_x, from_block_y, from_direction, figure);
	for (int n = 0; n < 4; n++) {
		for (int i = 0; i < 4; i++) {
			if(pieces[figure][to_direction][i][n] != 0 && (to_block_x + n > WIDTH_IN_BLOCKS - 1 ||
				  to_block_y - i > HIGHT_IN_BLOCKS - 1 || to_block_x + n < 0 ||
				  to_block_y - i < 0 ||  block_array[to_block_x + n][to_block_y - i] != 0)) {
				draw_figure(from_block_x, from_block_y, from_direction, figure,
							moving_piece);
				return false;
			}
		}
	}
	draw_figure(from_block_x, from_block_y, from_direction, figure,
				moving_piece);
	return true;
}

void move_block(int from_block_x, int from_block_y, int to_block_x,
				int to_block_y, enum rotation from_direction,
				enum rotation to_direction, enum figures figure,
				enum block_piece piece) {
	clean_figure(
		from_block_x - find_shift_x_of_center_square(from_direction, figure),
		from_block_y + find_shift_y_of_center_square(from_direction, figure),
		from_direction, figure);
	draw_figure(
		to_block_x - find_shift_x_of_center_square(to_direction, figure),
		to_block_y + find_shift_y_of_center_square(to_direction, figure),
		to_direction, figure, piece);
}

bool move_block_if_possible(int from_block_x, int from_block_y, int to_block_x,
						   int to_block_y, enum rotation from_direction,
						   enum rotation to_direction, enum figures figure) {
	if (is_move_possible(
			from_block_x -
				find_shift_x_of_center_square(from_direction, figure),
			from_block_y +
				find_shift_y_of_center_square(from_direction, figure),
			to_block_x - find_shift_x_of_center_square(to_direction, figure),
			to_block_y + find_shift_y_of_center_square(to_direction, figure),
			from_direction, to_direction, figure)) {
		move_block(from_block_x, from_block_y, to_block_x, to_block_y,
				   from_direction, to_direction, figure, moving_piece);
		return true;
	}
	return false;
}


void new_block() {
	current_x = WIDTH_IN_BLOCKS / 2 - 1;
	current_y = HIGHT_IN_BLOCKS - 2;
	current_figure = next_figure;
	next_figure = rand() % 7;
	current_rotation = up;
	future_rotation = up;
}

int check_for_filled_lines() {
	int i, n;
	for (n = 0; n < HIGHT_IN_BLOCKS; n++) {
		for (i = 0; i < WIDTH_IN_BLOCKS; i++) {
			if (block_array[i][n] == empty_piece) {
				break;
			}
		}
		if (i == WIDTH_IN_BLOCKS) {
			return n;
		}
	}
	return -1;
}

bool is_player_dead() {
	for (int i = 0; i < WIDTH_IN_BLOCKS; i++) {
		if (block_array[i][HIGHT_IN_BLOCKS - LINES_OF_DEATH_FROM_CELING] ==
			fixed_piece) {
			return true;
		}
	}
	return false;
}

void remove_filled_lines(int l) {
	for (int n = 0; n < WIDTH_IN_BLOCKS; n++) {
		block_array[n][l] = empty_piece;
	}
	for (int i = l + 1; i < HIGHT_IN_BLOCKS; i++) {
		for (int n = 0; n < WIDTH_IN_BLOCKS; n++) {
			if (block_array[n][i] == fixed_piece) {
				block_array[n][i - 1] = fixed_piece;
				block_array[n][i] = empty_piece;
			}
		}
	}
}

void change_color_if_settled() {
	if (!is_move_possible(current_x - find_shift_x_of_center_square(
										  current_rotation, current_figure),
						  current_y + find_shift_y_of_center_square(
										  current_rotation, current_figure),
						  current_x - find_shift_x_of_center_square(
										  current_rotation, current_figure),
						  current_y +
							  find_shift_y_of_center_square(current_rotation,
															current_figure) -
							  1,
						  current_rotation, current_rotation, current_figure)) {
		move_block(current_x, current_y, current_x, current_y, current_rotation,
				   current_rotation, current_figure, fixed_piece);
		new_block();
		int temp;
		while ((temp = check_for_filled_lines()) != -1) {
			remove_filled_lines(temp);
		}
	}
}

void draw_the_end() {
	gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLACK);
	gfx_textout(gfx_screenWidth() / 2 - 60, gfx_screenHeight() / 2 - 100,
				END_TEXT_LINE_1, WHITE);
	gfx_textout(gfx_screenWidth() / 2 - 50, gfx_screenHeight() / 2 - 50,
				END_TEXT_LINE_2, WHITE);
	gfx_updateScreen();
}

void new_game_conditions() {
	for (int i = 0; i < HIGHT_IN_BLOCKS; i++) {
		for (int n = 0; n < WIDTH_IN_BLOCKS; n++) {
			block_array[n][i] = 0;
		}
	}
	new_block();
	draw_all();
}

void one_move_down(){
			if (move_block_if_possible(current_x, current_y, current_x,
								   current_y - 1, current_rotation,
								   future_rotation, current_figure)) {
			current_y--;
		}
}

void one_move_right(){
			if (move_block_if_possible(current_x, current_y, current_x + 1,
								   current_y, current_rotation, future_rotation,
								   current_figure)) {
			current_x++;
		}
}

void one_move_left(){
			if (move_block_if_possible(current_x, current_y, current_x - 1,
								   current_y, current_rotation, future_rotation,
								   current_figure)) {
			current_x--;
		}
}

void one_turn_right(){
			future_rotation = (future_rotation + 1) % 4;
		if (move_block_if_possible(current_x, current_y, current_x, current_y,
								   current_rotation, future_rotation,
								   current_figure)) {
			current_rotation = future_rotation;
		} else {
			future_rotation = current_rotation;
		}
}
	
void game_movement() {
	change_color_if_settled();
	if (is_player_dead()) {
		new_game_conditions();
		draw_the_end();
		gfx_getkey();
		return;
	}
	if ((double)(clock() - start_time) / CLOCKS_PER_SEC > FALL_DELAY) {
		one_move_down();
		start_time = clock();
	}
	switch (gfx_pollkey()) {
	case SDLK_RIGHT:
		one_move_right();
		break;
	case SDLK_LEFT:
		one_move_left();
		break;
	case SDLK_DOWN:
		one_move_down();
		break;
	case ' ':
		one_turn_right();
		break;
	}
}

void initiate_all(){
	scale_size_of_structures();
	initiate_next_figure_array_coordinates();
	srand(time(NULL));
	initiate_block_array();
	new_block();
	draw_all();
	start_time = clock();
}

int main() {
	if (gfx_init()) {
		exit(3);
	}
	initiate_all();
	while (1) {
		game_movement();
		draw_all();
	}
	gfx_getkey();
	return 0;
}
