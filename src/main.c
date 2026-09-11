#include <graphics/drawing.h>
#include <graphics/color.h>
#include <graphics/text.h>
#include <graphics/lcdc.h>
#include <sh4a/input/keypad.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 528
#define SCREEN_HEIGHT 320
#define PADDLE_WIDTH 60
#define PADDLE_HEIGHT 8
#define BALL_SIZE 4
#define BLOCK_WIDTH 40
#define BLOCK_HEIGHT 10
#define BLOCK_ROWS 4
#define BLOCK_COLS 12

typedef struct {
  int x, y;
  int vx, vy;
} Ball;

typedef struct {
  int x, y;
  int active;
} Block;

Ball ball;
int paddle_x;
Block blocks[BLOCK_ROWS * BLOCK_COLS];
int score = 0;
int game_over = 0;

void init_game() {
  paddle_x = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
  ball.x = SCREEN_WIDTH / 2;
  ball.y = SCREEN_HEIGHT / 2;
  ball.vx = 3;
  ball.vy = -3;
  score = 0;
  game_over = 0;
  
  for (int i = 0; i < BLOCK_ROWS * BLOCK_COLS; i++) {
    blocks[i].active = 1;
    blocks[i].x = (i % BLOCK_COLS) * BLOCK_WIDTH + 8;
    blocks[i].y = (i / BLOCK_COLS) * BLOCK_HEIGHT + 20;
  }
}

void update_game() {
  keypad_read();
  
  // Move paddle
  if (get_key_state(KEY_LEFT) && paddle_x > 0) {
    paddle_x -= 5;
  }
  if (get_key_state(KEY_RIGHT) && paddle_x < SCREEN_WIDTH - PADDLE_WIDTH) {
    paddle_x += 5;
  }
  
  // Update ball position
  ball.x += ball.vx;
  ball.y += ball.vy;
  
  // Ball bounce off walls
  if (ball.x <= 0 || ball.x >= SCREEN_WIDTH - BALL_SIZE) {
    ball.vx = -ball.vx;
  }
  if (ball.y <= 0) {
    ball.vy = -ball.vy;
  }
  
  // Ball bounce off paddle
  if (ball.y + BALL_SIZE >= SCREEN_HEIGHT - PADDLE_HEIGHT - 5 &&
      ball.y <= SCREEN_HEIGHT - 5 &&
      ball.x + BALL_SIZE >= paddle_x &&
      ball.x <= paddle_x + PADDLE_WIDTH) {
    ball.vy = -ball.vy;
    score += 10;
  }
  
  // Ball out of bounds
  if (ball.y > SCREEN_HEIGHT) {
    game_over = 1;
  }
  
  // Block collision
  for (int i = 0; i < BLOCK_ROWS * BLOCK_COLS; i++) {
    if (!blocks[i].active) continue;
    
    if (ball.x + BALL_SIZE >= blocks[i].x &&
        ball.x <= blocks[i].x + BLOCK_WIDTH &&
        ball.y + BALL_SIZE >= blocks[i].y &&
        ball.y <= blocks[i].y + BLOCK_HEIGHT) {
      blocks[i].active = 0;
      ball.vy = -ball.vy;
      score += 100;
    }
  }
}

void draw_game() {
  // Clear screen
  set_pen(create_rgb16(0, 0, 0));
  draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  
  // Draw paddle
  set_pen(create_rgb16(0, 255, 0));
  draw_rect(paddle_x, SCREEN_HEIGHT - PADDLE_HEIGHT - 5, 
            paddle_x + PADDLE_WIDTH, SCREEN_HEIGHT - 5);
  
  // Draw ball
  set_pen(create_rgb16(255, 255, 255));
  draw_rect(ball.x, ball.y, ball.x + BALL_SIZE, ball.y + BALL_SIZE);
  
  // Draw blocks
  set_pen(create_rgb16(255, 0, 0));
  for (int i = 0; i < BLOCK_ROWS * BLOCK_COLS; i++) {
    if (blocks[i].active) {
      draw_rect(blocks[i].x, blocks[i].y,
                blocks[i].x + BLOCK_WIDTH, blocks[i].y + BLOCK_HEIGHT);
    }
  }
  
  // Draw score
  struct font *fnt = get_font();
  set_pen(create_rgb16(255, 255, 0));
  char score_str[32];
  sprintf(score_str, "Score: %d", score);
  render_text(10, 10, score_str);
  
  lcdc_copy_vram();
}

int main(void) {
  init_game();
  
  while (!game_over) {
    update_game();
    draw_game();
    
    if (get_key_state(KEY_POWER) || get_key_state(KEY_BACK)) {
      return -2;
    }
  }
  
  // Game over screen
  struct font *fnt = get_font();
  set_pen(create_rgb16(0, 0, 0));
  draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  set_pen(create_rgb16(255, 0, 0));
  render_text((SCREEN_WIDTH - (sizeof "GAME OVER" - 1) * fnt->width) / 2, 
              SCREEN_HEIGHT / 2 - fnt->height, "GAME OVER");
  set_pen(create_rgb16(255, 255, 255));
  char final_score[32];
  sprintf(final_score, "Final Score: %d", score);
  render_text((SCREEN_WIDTH - (strlen(final_score)) * fnt->width) / 2,
              SCREEN_HEIGHT / 2 + fnt->height, final_score);
  lcdc_copy_vram();
  
  while (1) {
    keypad_read();
    if (get_key_state(KEY_POWER) || get_key_state(KEY_BACK)) return -2;
  }
}
