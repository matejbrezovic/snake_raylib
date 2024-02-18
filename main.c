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
    unsigned int length;
    unsigned int curHeadIndex;
    unsigned int curTailIndex;
    Direction dir;
} Snake;

// Define global variables
static Snake snake;
static Vector2Int apple;
static bool gameOver = false;
static bool allowMove = true;
static unsigned int frameCounter = 0;
static Sound fxCollectApple = { 0 };
static Font font = { 0 };

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


int main(void) {
    InitWindow(2 * OFFSET + 2 * BORDER_LINE_WIDTH + COLS * CELL_SIZE, 2 * OFFSET + 2 * BORDER_LINE_WIDTH + ROWS * CELL_SIZE, "Snake");
    SetTargetFPS(TARGET_FPS);

    InitAudioDevice();
    fxCollectApple = LoadSound("resources/collectApple.wav");
    font = LoadFont("resources/SG08.ttf");

    InitGame();

    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }

    UnloadFont(font);
    UnloadSound(fxCollectApple);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}


void InitGame(void) {
    frameCounter = 0;
    gameOver = false;
    allowMove = true;
    InitSnake();
    GenerateNewApple();
}


void InitSnake(void) {
    snake.length = 1;
    snake.head.x = COLS / 2;
    snake.head.y = ROWS / 2;
    snake.tail = snake.head;
    snake.parts[0] = snake.head;
    snake.curHeadIndex = 0;
    snake.curTailIndex = 0;
    snake.dir = NONE;
}


// Draw 1 frame
void UpdateDrawFrame(void) {
    if (!gameOver) {
        UpdateDirection();
        if (snake.dir != NONE) {
            UpdateSnake();
        }
    } else {
        if (IsKeyPressed(KEY_ENTER)) {
            InitGame();
        }
    }
    
    gameOver = GameEnded();
    DrawGame();
    frameCounter++;
}


// 1 frame
void DrawGame(void) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    if (!gameOver){
        DrawTextEx(font, TextFormat("SCORE: %d", snake.length - 1), (Vector2) {10, 10}, 24, 2, BLACK);

        DrawRectangleLinesEx((Rectangle) {OFFSET - BORDER_LINE_WIDTH, OFFSET - BORDER_LINE_WIDTH, 
                                            COLS * CELL_SIZE + 2 * BORDER_LINE_WIDTH, ROWS * CELL_SIZE + 2 * BORDER_LINE_WIDTH}, 4, BLACK);

        // Draw apple
        DrawRectangleRounded((Rectangle) {OFFSET + apple.x * CELL_SIZE + (float) (CELL_SIZE - APPLE_SIZE) / 2, 
                                            OFFSET + apple.y * CELL_SIZE  + (float) (CELL_SIZE - APPLE_SIZE) / 2, APPLE_SIZE, APPLE_SIZE}, 0.8, 8, MAROON);

        // Draw snake
        for (int i = 0; i < snake.length; ++i) {
            if (snake.length % 2 == i % 2)
                DrawRectangleRounded((Rectangle) {OFFSET + snake.parts[(snake.curTailIndex + i) % (ROWS * COLS)].x * CELL_SIZE, 
                                                    OFFSET + snake.parts[(snake.curTailIndex + i) % (ROWS * COLS)].y * CELL_SIZE, CELL_SIZE, CELL_SIZE}, 0.6, 8, GREEN);
            else
                DrawRectangleRounded((Rectangle) {OFFSET + snake.parts[(snake.curTailIndex + i) % (ROWS * COLS)].x * CELL_SIZE, 
                                                    OFFSET + snake.parts[(snake.curTailIndex + i) % (ROWS * COLS)].y * CELL_SIZE, CELL_SIZE, CELL_SIZE}, 0.6, 8, DARKGREEN);
        }
    } else {
        DrawTextEx(font, "GAME OVER", (Vector2) {GetScreenWidth() / 2 - MeasureTextEx(font, "GAME OVER", 30, 2).x / 2, GetScreenHeight() / 2 - 60}, 30, 2, BLACK);
        DrawTextEx(font, TextFormat("SCORE: %03i", snake.length - 1), (Vector2) {GetScreenWidth() / 2 - MeasureTextEx(font, "SCORE: 000", 26, 2).x / 2, GetScreenHeight() / 2 - 35}, 26, 2, BLACK);
        DrawTextEx(font, "Press [Enter] to play again.", (Vector2) {GetScreenWidth() / 2 - MeasureTextEx(font, "Press [Enter] to play again.", 26, 1).x / 2, GetScreenHeight() / 2}, 26, 1, DARKGREEN);
    }

    EndDrawing();
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
        PlaySound(fxCollectApple);
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
