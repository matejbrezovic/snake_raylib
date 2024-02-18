#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "raylib.h"

#define ROWS 30
#define COLS 30
#define OFFSET 70
#define BORDER_LINE_WIDTH 4
#define CELL_SIZE 20
#define APPLE_SIZE 14
#define TARGET_FPS 60


typedef struct {
    int x;
    int y;
} Vector2Int;


typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
} Direction;


typedef struct {
    Vector2Int head;
    Vector2Int tail;
    Vector2Int parts[ROWS * COLS];
    int length;
    int curHeadIndex;
    int curTailIndex;
    Direction dir;
} Snake;

// Define global variables
static Snake snake;
static Vector2Int apple;
static bool gameOver = false;
static bool paused = false;
static bool allowMove = false;
static int frameCounter = 0;

static char text[50];

// Declare functions
void InitSnake(void);
void UpdateDirection(void);
void UpdateSnake(void);
bool GameEnded(void);
bool IsSnakeSegment(const int x, const int y);
bool HasSnakeCollidedWithItself(void);
bool HasSnakeCollidedWithApple(void);
void GenerateNewApple(void);
void InitGame(void);
void UpdateDrawFrame(void);
void UpdateGame(void);
void DrawGame(void);
bool hasTicked(const float tickDuration);


int main(void) {
    InitWindow(2 * OFFSET + 2 * BORDER_LINE_WIDTH + COLS * CELL_SIZE, 2 * OFFSET + 2 * BORDER_LINE_WIDTH + ROWS * CELL_SIZE, "Snake");
    SetTargetFPS(TARGET_FPS);

    InitGame();


    while (!WindowShouldClose()) {
        UpdateDirection();
        if (snake.dir != NONE) {
            UpdateDrawFrame();
        }
    }

    CloseWindow();

    return 0;
}


bool hasTicked(const float tickDuration) {
    float time = floorf((float)GetTime() / tickDuration);
    static float currentTime = 0;
    if (currentTime != time)
    {
        currentTime = time;
        return true;
    }
    return false;
}


void InitGame(void) {
    frameCounter = 0;
    gameOver = false;
    allowMove = false;
    InitSnake();
    GenerateNewApple();
}

// Draw 1 frame
void UpdateDrawFrame(void) {
    UpdateGame();
    DrawGame();
    frameCounter++;
}


void UpdateGame(void) {
    UpdateSnake();
    
    if (GameEnded()) {
        gameOver = true;
        return;
    }


}

// 1 frame
void DrawGame(void) {
    BeginDrawing(); 
    
    sprintf(text, "HEAD: %d %d\nLENGTH: %d\nGAMEOVER: %d", snake.head.x, snake.head.y, snake.length, 0x1 & gameOver);
    DrawText(text, 10, 10, 20, BLACK);

    ClearBackground((Color) {150, 190, 37});
    DrawRectangleLinesEx((Rectangle) {OFFSET - BORDER_LINE_WIDTH, OFFSET - BORDER_LINE_WIDTH, COLS * CELL_SIZE + 2 * BORDER_LINE_WIDTH, ROWS * CELL_SIZE + 2 * BORDER_LINE_WIDTH}, 4, BLACK);

    // Draw apple
    DrawRectangleRounded((Rectangle) {OFFSET + apple.x * CELL_SIZE + (float) (CELL_SIZE - APPLE_SIZE) / 2, 
                                        OFFSET + apple.y * CELL_SIZE  + (float) (CELL_SIZE - APPLE_SIZE) / 2, APPLE_SIZE, APPLE_SIZE}, 0.5, 6, MAROON);

    // Draw snake
    for (int i = 0; i < snake.length; ++i) {
        if (snake.length % 2 == i % 2)
            DrawRectangleRounded((Rectangle) {OFFSET + snake.parts[(snake.curTailIndex + i) % (ROWS * COLS)].x * CELL_SIZE, 
                                                OFFSET + snake.parts[(snake.curTailIndex + i) % (ROWS * COLS)].y * CELL_SIZE, CELL_SIZE, CELL_SIZE}, 0.5, 6, BLUE);
        else
            DrawRectangleRounded((Rectangle) {OFFSET + snake.parts[(snake.curTailIndex + i) % (ROWS * COLS)].x * CELL_SIZE, 
                                                OFFSET + snake.parts[(snake.curTailIndex + i) % (ROWS * COLS)].y * CELL_SIZE, CELL_SIZE, CELL_SIZE}, 0.5, 6, DARKBLUE);
    }

    EndDrawing();
}


void InitSnake(void) {
    snake.length = 1;
    snake.head.x = COLS / 2;
    snake.head.y = ROWS / 2;
    snake.tail = snake.head;
    snake.parts[0] = snake.head;
    snake.curHeadIndex = 0;
    snake.curTailIndex = 0;
    snake.dir = RIGHT;
}


void UpdateSnake(void) {    
    if (frameCounter % (int)(TARGET_FPS * 0.15) != 0) {
        return;
    }
    snake.curHeadIndex = (snake.curHeadIndex + 1) % (ROWS * COLS);

    switch (snake.dir) {
        case LEFT:
            snake.head.x -= 1;
            break;
        case RIGHT:
            snake.head.x += 1;
            break;
        case UP:
            snake.head.y -= 1;
            break;
        case DOWN:
            snake.head.y += 1;
            break;
        case NONE:
            break;
    }
    snake.parts[snake.curHeadIndex] = snake.head;

    if (!HasSnakeCollidedWithApple()) {
        snake.curTailIndex = (snake.curTailIndex + 1) % (ROWS * COLS);
        snake.tail = snake.parts[snake.curTailIndex];
    } else {
        snake.length++;
        GenerateNewApple();
    }
    allowMove = true;
}


void UpdateDirection(void) {
    if ((IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) && snake.dir != RIGHT && allowMove) {
        snake.dir = LEFT;
        allowMove = false;
    } else if ((IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) && snake.dir != LEFT && allowMove) {
        snake.dir = RIGHT;
        allowMove = false;
    } else if ((IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && snake.dir != DOWN && allowMove) {
        snake.dir = UP;
        allowMove = false;
    } else if ((IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) && snake.dir != UP && allowMove) {
        snake.dir = DOWN;
        allowMove = false;
    }
}


void GenerateNewApple(void) {
    do {
        apple.x = rand() % COLS;
        apple.y = rand() % ROWS;
    } while (IsSnakeSegment(apple.x, apple.y));
}


bool HasSnakeCollidedWithApple(void) {
    return (snake.head.x == apple.x && 
            snake.head.y == apple.y);
}


bool GameEnded(void) {
    return (snake.head.x < 0 || snake.head.x > COLS ||
            snake.head.y < 0 || snake.head.y > ROWS ||
            HasSnakeCollidedWithItself());
}


bool HasSnakeCollidedWithItself(void) {
    for (int i = 0; i < snake.length - 1; ++i) {
        if (snake.head.x == snake.parts[(snake.curTailIndex + i) % (ROWS * COLS)].x && 
            snake.head.y == snake.parts[(snake.curTailIndex + i) % (ROWS * COLS)].y)
            return true;
    }
    return false;
}


bool IsSnakeSegment(const int x, const int y) {
    for (int i = 0; i < snake.length; ++i) {
        if (x == snake.parts[(snake.curTailIndex + i) % (ROWS * COLS)].x && 
            y == snake.parts[(snake.curTailIndex + i) % (ROWS * COLS)].y)
            return true;
    }
    return false;
}
