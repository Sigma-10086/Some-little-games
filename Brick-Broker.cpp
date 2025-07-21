#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <cstdio>
#include <windows.h>

// 参数设置
#define WIDTH 800
#define HEIGHT 600
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 15
#define BALL_RADIUS 10
#define BRICK_ROWS 6
#define BRICK_COLS 10
#define BRICK_WIDTH 70
#define BRICK_HEIGHT 20
#define BRICK_GAP 5
#define MAX_BALLS 3  // 最大球数量
#define MAX_SPIKES 8  // 最大刺数量
#define SKILL_COOLDOWN_1 5000  // 倍化技能冷却时间(毫秒)
#define SKILL_COOLDOWN_2 5000  // 分裂技能冷却时间(毫秒)
#define SKILL_COOLDOWN_3 30000 // 绽放技能冷却时间(毫秒)

// 颜色定义
#define COLOR_BG RGB(0, 0, 0)
#define COLOR_PADDLE RGB(255, 255, 255)
#define COLOR_BALL RGB(255, 255, 0)
#define COLOR_BRICKS RGB(255, 0, 0)
#define COLOR_TEXT RGB(255, 255, 255)
#define COLOR_BALL_SPLIT1 RGB(0, 255, 0)  // 分裂球颜色1
#define COLOR_BALL_SPLIT2 RGB(0, 0, 255)  // 分裂球颜色2
#define COLOR_SPIKE RGB(255, 165, 0)      // 刺的颜色
#define COLOR_SKILL_READY RGB(0, 255, 0)  // 技能就绪颜色
#define COLOR_SKILL_COOLDOWN RGB(255, 165, 0)  // 技能冷却颜色

// 对象结构体
typedef struct {
    int x, y;           // 位置
    int width, height;  // 尺寸
    int dx, dy;         // 速度
    COLORREF color;     // 颜色
} Paddle;

typedef struct {
    int x, y;           // 位置
    int radius;         // 半径
    int dx, dy;         // 速度
    COLORREF color;     // 颜色
    bool active;        // 是否存在
    bool isSplit;       // 是否为分裂球
} Ball;

typedef struct {
    int x, y;           // 位置
    int width, height;  // 尺寸
    COLORREF color;     // 颜色
    bool active;        // 是否存在
} Brick;

typedef struct {
    int x, y;           // 位置
    int length;         // 长度
    int dx, dy;         // 速度
    COLORREF color;     // 颜色
    bool active;        // 是否存在
} Spike;

// 技能结构体
typedef struct {
    bool active;        // 技能是否激活
    bool onCooldown;    // 技能是否在冷却中
    DWORD startTime;    // 技能开始时间
    DWORD duration;     // 技能持续时间（毫秒）
    DWORD cooldownEnd;  // 冷却结束时间
    COLORREF color;     // 技能显示颜色
} Skill;

// 游戏状态
typedef enum {
    GAME_START,
    GAME_PLAYING,
    GAME_PAUSED,
    GAME_OVER,
    GAME_WON
} GameState;

// 全局变量
Paddle paddle;
Ball balls[MAX_BALLS];  // 支持多个球
int ballCount = 1;      // 当前球数量
Brick bricks[BRICK_ROWS][BRICK_COLS];
Spike spikes[MAX_SPIKES]; // 刺数组
GameState gameState = GAME_START;
int score = 0;
int lives = 3;
Skill enlargeSkill;     // 倍化技能
Skill splitSkill;       // 分裂技能
Skill bloomSkill;       // 绽放技能

// 函数声明
void initGame();
void initPaddle();
void initBall();
void initBricks();
void initSpikes();      // 初始化刺
void drawGame();
void drawPaddle();
void drawBalls();
void drawBricks();
void drawSpikes();      // 绘制刺
void drawText();
void updateGame();
void updatePaddle();
void updateBalls();
void updateSpikes();    // 更新刺的位置
void checkCollisions();
void checkBrickCollisions(int ballIndex);
void checkPaddleCollision(int ballIndex);
void checkWallCollision(int ballIndex);
void checkSpikeCollisions(); // 检查刺与砖块的碰撞
void handleInput();
void resetBall();
void gameOver();
void gameWon();
void activateEnlargeSkill();  // 激活倍化技能
void activateSplitSkill();    // 激活分裂技能
void activateBloomSkill();    // 激活绽放技能
void updateSkills();          // 更新技能状态
bool isSkillReady(Skill* skill);  // 检查技能是否就绪
bool LineIntersectRect(int x1, int y1, int x2, int y2, int rx1, int ry1, int rx2, int ry2);
bool LineIntersectLine(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);

// 初始化
void initGame() {
    initgraph(WIDTH, HEIGHT);
    srand(time(NULL));
    initPaddle();
    initBall();
    initBricks();
    initSpikes(); // 初始化刺
    setbkmode(TRANSPARENT);  // 设置背景透明
    BeginBatchDraw();  // 启用双缓冲绘图
    gameState = GAME_START;

    // 初始化技能
    enlargeSkill.active = false;
    enlargeSkill.onCooldown = false;
    enlargeSkill.color = COLOR_SKILL_READY;

    splitSkill.active = false;
    splitSkill.onCooldown = false;
    splitSkill.color = COLOR_SKILL_READY;

    bloomSkill.active = false;
    bloomSkill.onCooldown = false;
    bloomSkill.color = COLOR_SKILL_READY;
}

// 初始化挡板
void initPaddle() {
    paddle.x = (WIDTH - PADDLE_WIDTH) / 2;
    paddle.y = HEIGHT - PADDLE_HEIGHT - 20;
    paddle.width = PADDLE_WIDTH;
    paddle.height = PADDLE_HEIGHT;
    paddle.dx = 0;
    paddle.color = COLOR_PADDLE;
}

// 初始化球
void initBall() {
    // 重置所有球
    for (int i = 0; i < MAX_BALLS; i++) {
        balls[i].active = false;
    }

    // 初始化主球
    balls[0].x = WIDTH / 2;
    balls[0].y = HEIGHT / 2;
    balls[0].radius = BALL_RADIUS;
    balls[0].dx = 3 * (rand() % 2 == 0 ? 1 : -1);
    balls[0].dy = -3;
    balls[0].color = COLOR_BALL;
    balls[0].active = true;
    balls[0].isSplit = false;

    ballCount = 1;
}

// 初始化砖块
void initBricks() {
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            bricks[i][j].x = j * (BRICK_WIDTH + BRICK_GAP) + (WIDTH - (BRICK_WIDTH + BRICK_GAP) * BRICK_COLS) / 2;
            bricks[i][j].y = i * (BRICK_HEIGHT + BRICK_GAP) + 50;
            bricks[i][j].width = BRICK_WIDTH;
            bricks[i][j].height = BRICK_HEIGHT;
            bricks[i][j].color = COLOR_BRICKS;
            bricks[i][j].active = true;
        }
    }
}

// 初始化刺
void initSpikes() {
    for (int i = 0; i < MAX_SPIKES; i++) {
        spikes[i].active = false;
    }
}

// 绘制游戏画面
void drawGame() {
    cleardevice();
    setbkcolor(COLOR_BG);
    drawPaddle();
    drawBalls();
    drawBricks();
    drawSpikes(); // 绘制刺
    drawText();
    FlushBatchDraw();  // 批量绘制
}

// 绘制挡板
void drawPaddle() {
    setfillcolor(paddle.color);
    bar(paddle.x, paddle.y, paddle.x + paddle.width, paddle.y + paddle.height);
}

// 绘制球
void drawBalls() {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (balls[i].active) {
            setfillcolor(balls[i].color);
            solidcircle(balls[i].x, balls[i].y, balls[i].radius);
        }
    }
}

// 绘制砖块
void drawBricks() {
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (bricks[i][j].active) {
                setfillcolor(bricks[i][j].color);
                bar(bricks[i][j].x, bricks[i][j].y,
                    bricks[i][j].x + bricks[i][j].width,
                    bricks[i][j].y + bricks[i][j].height);
            }
        }
    }
}

// 绘制刺
void drawSpikes() {
    for (int i = 0; i < MAX_SPIKES; i++) {
        if (spikes[i].active) {
            setlinecolor(spikes[i].color);
            setlinestyle(PS_SOLID, 2);
            line(spikes[i].x, spikes[i].y,
                spikes[i].x + spikes[i].dx * spikes[i].length,
                spikes[i].y + spikes[i].dy * spikes[i].length);
        }
    }
}

// 绘制文字信息
void drawText() {
    settextstyle(20, 0, _T("宋体"));

    // 分数和生命值
    TCHAR scoreText[50];
    _stprintf_s(scoreText, _countof(scoreText), _T("分数: %d"), score);
    outtextxy(10, 10, scoreText);

    TCHAR livesText[50];
    _stprintf_s(livesText, _countof(livesText), _T("生命: %d"), lives);
    outtextxy(WIDTH - 100, 10, livesText);

    // 技能状态
    DWORD currentTime = GetTickCount();

    // 倍化技能
    if (enlargeSkill.active) {
        DWORD remaining = (enlargeSkill.startTime + enlargeSkill.duration - currentTime) / 1000;
        TCHAR skillText[50];
        _stprintf_s(skillText, _countof(skillText), _T("技能: 倍化 (%d秒)"), remaining);
        settextcolor(COLOR_SKILL_READY);
        outtextxy(10, 40, skillText);
    }
    else if (enlargeSkill.onCooldown) {
        DWORD remaining = (enlargeSkill.cooldownEnd - currentTime) / 1000;
        TCHAR skillText[50];
        _stprintf_s(skillText, _countof(skillText), _T("技能: 倍化 (冷却中: %d秒)"), remaining);
        settextcolor(COLOR_SKILL_COOLDOWN);
        outtextxy(10, 40, skillText);
    }
    else {
        TCHAR skillText[50];
        _stprintf_s(skillText, _countof(skillText), _T("技能: 倍化 (就绪)"));
        settextcolor(COLOR_SKILL_READY);
        outtextxy(10, 40, skillText);
    }

    // 分裂技能
    if (splitSkill.active) {
        DWORD remaining = (splitSkill.startTime + splitSkill.duration - currentTime) / 1000;
        TCHAR skillText[50];
        _stprintf_s(skillText, _countof(skillText), _T("技能: 分裂 (%d秒)"), remaining);
        settextcolor(COLOR_SKILL_READY);
        outtextxy(10, 70, skillText);
    }
    else if (splitSkill.onCooldown) {
        DWORD remaining = (splitSkill.cooldownEnd - currentTime) / 1000;
        TCHAR skillText[50];
        _stprintf_s(skillText, _countof(skillText), _T("技能: 分裂 (冷却中: %d秒)"), remaining);
        settextcolor(COLOR_SKILL_COOLDOWN);
        outtextxy(10, 70, skillText);
    }
    else {
        TCHAR skillText[50];
        _stprintf_s(skillText, _countof(skillText), _T("技能: 分裂 (就绪)"));
        settextcolor(COLOR_SKILL_READY);
        outtextxy(10, 70, skillText);
    }

    // 绽放技能
    if (bloomSkill.active) {
        TCHAR skillText[50];
        _stprintf_s(skillText, _countof(skillText), _T("技能: 绽放 (发射中)"));
        settextcolor(COLOR_SKILL_READY);
        outtextxy(10, 100, skillText);
    }
    else if (bloomSkill.onCooldown) {
        DWORD remaining = (bloomSkill.cooldownEnd - currentTime) / 1000;
        TCHAR skillText[50];
        _stprintf_s(skillText, _countof(skillText), _T("技能: 绽放 (冷却中: %d秒)"), remaining);
        settextcolor(COLOR_SKILL_COOLDOWN);
        outtextxy(10, 100, skillText);
    }
    else {
        TCHAR skillText[50];
        _stprintf_s(skillText, _countof(skillText), _T("技能: 绽放 (就绪)"));
        settextcolor(COLOR_SKILL_READY);
        outtextxy(10, 100, skillText);
    }

    // 游戏状态提示
    if (gameState == GAME_START) {
        settextstyle(40, 0, _T("宋体"));
        outtextxy(WIDTH / 2 - 150, HEIGHT / 2 - 20, _T("按空格键开始游戏"));
        settextstyle(20, 0, _T("宋体"));
        outtextxy(WIDTH / 2 - 200, HEIGHT / 2 + 30, _T("按Z键: 倍化 (半径×3, 冷却5秒)"));
        outtextxy(WIDTH / 2 - 200, HEIGHT / 2 + 60, _T("按X键: 分裂 (冷却5秒)"));
        outtextxy(WIDTH / 2 - 200, HEIGHT / 2 + 90, _T("按C键: 绽放 (冷却30秒)"));
    }
    else if (gameState == GAME_PAUSED) {
        settextstyle(40, 0, _T("宋体"));
        outtextxy(WIDTH / 2 - 100, HEIGHT / 2 - 20, _T("游戏暂停"));
        outtextxy(WIDTH / 2 - 150, HEIGHT / 2 + 30, _T("按空格键继续"));
    }
    else if (gameState == GAME_OVER) {
        settextstyle(40, 0, _T("宋体"));
        outtextxy(WIDTH / 2 - 100, HEIGHT / 2 - 20, _T("游戏结束"));
        TCHAR finalScore[50];
        _stprintf_s(finalScore, _countof(finalScore), _T("最终得分: %d"), score);
        outtextxy(WIDTH / 2 - 100, HEIGHT / 2 + 30, finalScore);
        outtextxy(WIDTH / 2 - 150, HEIGHT / 2 + 80, _T("按R键重新开始"));
    }
    else if (gameState == GAME_WON) {
        settextstyle(40, 0, _T("宋体"));
        outtextxy(WIDTH / 2 - 100, HEIGHT / 2 - 20, _T("恭喜胜利!"));
        TCHAR finalScore[50];
        _stprintf_s(finalScore, _countof(finalScore), _T("最终得分: %d"), score);
        outtextxy(WIDTH / 2 - 100, HEIGHT / 2 + 30, finalScore);
        outtextxy(WIDTH / 2 - 150, HEIGHT / 2 + 80, _T("按R键重新开始"));
    }
}
// 更新游戏状态
void updateGame() {
    if (gameState == GAME_PLAYING) {
        updatePaddle();
        updateBalls();
        updateSpikes(); // 更新刺的位置
        checkCollisions();
        checkSpikeCollisions(); // 检查刺与砖块的碰撞
        updateSkills();  // 更新技能状态
    }
}

// 更新挡板位置
void updatePaddle() {
    paddle.x += paddle.dx;

    // 边界检查
    if (paddle.x < 0) {
        paddle.x = 0;
    }
    else if (paddle.x + paddle.width > WIDTH) {
        paddle.x = WIDTH - paddle.width;
    }
}

// 更新球的位置
void updateBalls() {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (balls[i].active) {
            balls[i].x += balls[i].dx;
            balls[i].y += balls[i].dy;
        }
    }
}

// 更新刺的位置
void updateSpikes() {
    for (int i = 0; i < MAX_SPIKES; i++) {
        if (spikes[i].active) {
            spikes[i].x += spikes[i].dx;
            spikes[i].y += spikes[i].dy;

            // 检查是否超出屏幕范围
            if (spikes[i].x < 0 || spikes[i].x > WIDTH ||
                spikes[i].y < 0 || spikes[i].y > HEIGHT) {
                spikes[i].active = false;
            }
        }
    }
}

// 检查碰撞
void checkCollisions() {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (balls[i].active) {
            checkBrickCollisions(i);
            checkPaddleCollision(i);
            checkWallCollision(i);
        }
    }
}

// 检查与砖块的碰撞
void checkBrickCollisions(int ballIndex) {
    Ball* ball = &balls[ballIndex];

    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (bricks[i][j].active) {
                // 检查球与砖块的碰撞
                if (ball->x + ball->radius > bricks[i][j].x &&
                    ball->x - ball->radius < bricks[i][j].x + bricks[i][j].width &&
                    ball->y + ball->radius > bricks[i][j].y &&
                    ball->y - ball->radius < bricks[i][j].y + bricks[i][j].height) {

                    // 计算碰撞方向
                    int overlapLeft = ball->x + ball->radius - bricks[i][j].x;
                    int overlapRight = bricks[i][j].x + bricks[i][j].width - (ball->x - ball->radius);
                    int overlapTop = ball->y + ball->radius - bricks[i][j].y;
                    int overlapBottom = bricks[i][j].y + bricks[i][j].height - (ball->y - ball->radius);

                    // 找出最小重叠方向
                    int minOverlap = min(min(overlapLeft, overlapRight), min(overlapTop, overlapBottom));

                    if (minOverlap == overlapLeft) {
                        ball->dx = -abs(ball->dx);  // 向左反弹
                    }
                    else if (minOverlap == overlapRight) {
                        ball->dx = abs(ball->dx);   // 向右反弹
                    }
                    else if (minOverlap == overlapTop) {
                        ball->dy = -abs(ball->dy);  // 向上反弹
                    }
                    else {
                        ball->dy = abs(ball->dy);   // 向下反弹
                    }

                    // 砖块被击中
                    bricks[i][j].active = false;
                    score += 10;

                    // 增加球速
                    if (ball->dx > 0) ball->dx = min(ball->dx + 0.1, 8);
                    else ball->dx = max(ball->dx - 0.1, -8);

                    if (ball->dy > 0) ball->dy = min(ball->dy + 0.1, 8);
                    else ball->dy = max(ball->dy - 0.1, -8);

                    return;  // 一个球一次只击碎一个砖块
                }
            }
        }
    }

    // 检查是否所有砖块都被摧毁
    bool allBricksDestroyed = true;
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (bricks[i][j].active) {
                allBricksDestroyed = false;
                break;
            }
        }
        if (!allBricksDestroyed) break;
    }

    if (allBricksDestroyed) {
        gameWon();
    }
}

// 检查与挡板的碰撞
void checkPaddleCollision(int ballIndex) {
    Ball* ball = &balls[ballIndex];

    if (ball->y + ball->radius > paddle.y &&
        ball->y - ball->radius < paddle.y + paddle.height &&
        ball->x + ball->radius > paddle.x &&
        ball->x - ball->radius < paddle.x + paddle.width) {

        // 计算球在挡板上的相对位置
        float relativeIntersectX = (paddle.x + paddle.width / 2) - ball->x;
        float normalizedRelativeIntersectionX = (relativeIntersectX / (paddle.width / 2));
        float bounceAngle = normalizedRelativeIntersectionX * 3.14159 / 3;  // 最大反弹角度为60度

        // 更新球的速度
        ball->dx = -5 * sin(bounceAngle);
        ball->dy = -5 * cos(bounceAngle);
    }
}

// 检查与墙壁的碰撞
void checkWallCollision(int ballIndex) {
    Ball* ball = &balls[ballIndex];

    // 左边界
    if (ball->x - ball->radius < 0) {
        ball->x = ball->radius;
        ball->dx = -ball->dx;
    }
    // 右边界
    else if (ball->x + ball->radius > WIDTH) {
        ball->x = WIDTH - ball->radius;
        ball->dx = -ball->dx;
    }
    // 上边界
    if (ball->y - ball->radius < 0) {
        ball->y = ball->radius;
        ball->dy = -ball->dy;
    }
    // 下边界（游戏结束）
    else if (ball->y + ball->radius > HEIGHT) {
        if (!ball->isSplit) {  // 分裂球不掉血
            lives--;
            if (lives <= 0) {
                gameOver();
            }
            else {
                resetBall();
            }
        }
        else {
            ball->active = false;  // 分裂球消失

            // 检查是否所有球都消失了
            bool allBallsLost = true;
            for (int i = 0; i < MAX_BALLS; i++) {
                if (balls[i].active) {
                    allBallsLost = false;
                    break;
                }
            }

            if (allBallsLost) {
                resetBall();
            }
        }
    }
}

// 检查刺与砖块的碰撞
void checkSpikeCollisions() {
    for (int i = 0; i < MAX_SPIKES; i++) {
        if (spikes[i].active) {
            for (int j = 0; j < BRICK_ROWS; j++) {
                for (int k = 0; k < BRICK_COLS; k++) {
                    if (bricks[j][k].active) {
                        // 计算刺的终点
                        int endX = spikes[i].x + spikes[i].dx * spikes[i].length;
                        int endY = spikes[i].y + spikes[i].dy * spikes[i].length;

                        // 检查线段与矩形碰撞
                        bool collision = false;

                        // 检查刺的起点是否在砖块内
                        if (spikes[i].x >= bricks[j][k].x && spikes[i].x <= bricks[j][k].x + bricks[j][k].width &&
                            spikes[i].y >= bricks[j][k].y && spikes[i].y <= bricks[j][k].y + bricks[j][k].height) {
                            collision = true;
                        }
                        // 检查刺的终点是否在砖块内
                        else if (endX >= bricks[j][k].x && endX <= bricks[j][k].x + bricks[j][k].width &&
                            endY >= bricks[j][k].y && endY <= bricks[j][k].y + bricks[j][k].height) {
                            collision = true;
                        }
                        // 检查线段与砖块边界的交点
                        else {
                            // 简化的线段与矩形碰撞检测
                            if (LineIntersectRect(spikes[i].x, spikes[i].y, endX, endY,
                                bricks[j][k].x, bricks[j][k].y,
                                bricks[j][k].x + bricks[j][k].width,
                                bricks[j][k].y + bricks[j][k].height)) {
                                collision = true;
                            }
                        }

                        if (collision) {
                            bricks[j][k].active = false;
                            spikes[i].active = false;
                            score += 10;
                            break;
                        }
                    }
                }
                if (!spikes[i].active) break;  // 如果刺已经消失，跳出循环
            }
        }
    }
}

// 检查线段与矩形是否相交
bool LineIntersectRect(int x1, int y1, int x2, int y2, int rx1, int ry1, int rx2, int ry2) {
    // 快速排斥试验
    if (x1 > rx2 && x2 > rx2) return false;
    if (x1 < rx1 && x2 < rx1) return false;
    if (y1 > ry2 && y2 > ry2) return false;
    if (y1 < ry1 && y2 < ry1) return false;

    // 线段与矩形四条边的交点检测
    if (LineIntersectLine(x1, y1, x2, y2, rx1, ry1, rx1, ry2)) return true; // 左边界
    if (LineIntersectLine(x1, y1, x2, y2, rx2, ry1, rx2, ry2)) return true; // 右边界
    if (LineIntersectLine(x1, y1, x2, y2, rx1, ry1, rx2, ry1)) return true; // 上边界
    if (LineIntersectLine(x1, y1, x2, y2, rx1, ry2, rx2, ry2)) return true; // 下边界

    // 线段的两个端点都在矩形内
    if (x1 >= rx1 && x1 <= rx2 && y1 >= ry1 && y1 <= ry2) return true;
    if (x2 >= rx1 && x2 <= rx2 && y2 >= ry1 && y2 <= ry2) return true;

    return false;
}

// 检查两条线段是否相交
bool LineIntersectLine(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
    // 计算叉积
    float denominator = (x2 - x1) * (y4 - y3) - (y2 - y1) * (x4 - x3);

    // 如果分母为0，表示线段平行或共线
    if (denominator == 0) {
        return false;
    }

    // 计算参数t和u
    float t = ((x3 - x1) * (y4 - y3) - (y3 - y1) * (x4 - x3)) / denominator;
    float u = ((x3 - x1) * (y2 - y1) - (y3 - y1) * (x2 - x1)) / denominator;

    // 如果t和u都在[0,1]范围内，表示两条线段相交
    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
        return true;
    }

    return false;
}

// 处理用户输入
void handleInput() {
    paddle.dx = 0;

    // 处理方向键（A/左箭头和D/右箭头）
    if (GetAsyncKeyState('A') < 0 || GetAsyncKeyState(VK_LEFT) < 0) {
        paddle.dx = -5;
    }
    else if (GetAsyncKeyState('D') < 0 || GetAsyncKeyState(VK_RIGHT) < 0) {
        paddle.dx = 5;
    }

    // 处理空格键（开始/暂停游戏）
    static bool spacePressed = false;
    if (GetAsyncKeyState(VK_SPACE) < 0) {
        if (!spacePressed) {
            spacePressed = true;
            if (gameState == GAME_START || gameState == GAME_PAUSED) {
                gameState = GAME_PLAYING;
            }
            else if (gameState == GAME_PLAYING) {
                gameState = GAME_PAUSED;
            }
        }
    }
    else {
        spacePressed = false;
    }

    // 处理R键（重新开始游戏）
    static bool rPressed = false;
    if (GetAsyncKeyState('R') < 0 || GetAsyncKeyState('r') < 0) {
        if (!rPressed) {
            rPressed = true;
            initGame();
            score = 0;
            lives = 3;
            gameState = GAME_START;
        }
    }
    else {
        rPressed = false;
    }

    // 处理Z键（倍化技能）
    static bool zPressed = false;
    if (GetAsyncKeyState('Z') < 0 || GetAsyncKeyState('z') < 0) {
        if (!zPressed && gameState == GAME_PLAYING && isSkillReady(&enlargeSkill)) {
            zPressed = true;
            activateEnlargeSkill();
        }
    }
    else {
        zPressed = false;
    }

    // 处理X键（分裂技能）
    static bool xPressed = false;
    if (GetAsyncKeyState('X') < 0 || GetAsyncKeyState('x') < 0) {
        if (!xPressed && gameState == GAME_PLAYING && isSkillReady(&splitSkill)) {
            xPressed = true;
            activateSplitSkill();
        }
    }
    else {
        xPressed = false;
    }

    // 处理C键（绽放技能）
    static bool cPressed = false;
    if (GetAsyncKeyState('C') < 0 || GetAsyncKeyState('c') < 0) {
        if (!cPressed && gameState == GAME_PLAYING && isSkillReady(&bloomSkill)) {
            cPressed = true;
            activateBloomSkill();
        }
    }
    else {
        cPressed = false;
    }
}

// 检查技能是否就绪
bool isSkillReady(Skill* skill) {
    return !skill->active && !skill->onCooldown;
}

// 激活倍化技能
void activateEnlargeSkill() {
    enlargeSkill.active = true;
    enlargeSkill.onCooldown = true;
    enlargeSkill.startTime = GetTickCount();
    enlargeSkill.duration = 10000;  // 10秒持续时间
    enlargeSkill.cooldownEnd = enlargeSkill.startTime + enlargeSkill.duration + SKILL_COOLDOWN_1;

    // 放大所有活跃的球（半径×3）
    for (int i = 0; i < MAX_BALLS; i++) {
        if (balls[i].active) {
            balls[i].radius = BALL_RADIUS * 3;
        }
    }
}

// 激活分裂技能
void activateSplitSkill() {
    splitSkill.active = true;
    splitSkill.onCooldown = true;
    splitSkill.startTime = GetTickCount();
    splitSkill.duration = 10000;  // 10秒持续时间
    splitSkill.cooldownEnd = splitSkill.startTime + splitSkill.duration + SKILL_COOLDOWN_2;

    // 分裂球逻辑
    for (int i = 0; i < MAX_BALLS; i++) {
        if (balls[i].active && !balls[i].isSplit) {  // 只分裂主球
            Ball* mainBall = &balls[i];

    
                // 创建第一个分裂球
                for (int j = 0; j < MAX_BALLS; j++) {
                    if (!balls[j].active) {
                        balls[j].x = mainBall->x;
                        balls[j].y = mainBall->y;
                        balls[j].radius = mainBall->radius;
                        balls[j].dx = mainBall->dx * 0.8;  // 速度稍慢
                        balls[j].dy = mainBall->dy * 0.8;
                        balls[j].color = COLOR_BALL_SPLIT1;
                        balls[j].active = true;
                        balls[j].isSplit = true;
                        ballCount++;
                        break;
                    }
                }

                // 创建第二个分裂球
                for (int j = 0; j < MAX_BALLS; j++) {
                    if (!balls[j].active) {
                        balls[j].x = mainBall->x;
                        balls[j].y = mainBall->y;
                        balls[j].radius = mainBall->radius;
                        balls[j].dx = -mainBall->dx * 0.8;  // 反向速度
                        balls[j].dy = mainBall->dy * 0.8;
                        balls[j].color = COLOR_BALL_SPLIT2;
                        balls[j].active = true;
                        balls[j].isSplit = true;
                        ballCount++;
                        break;
                    }
                }

                break;  // 只分裂一个主球
            }
        }
    }

    // 激活绽放技能
    void activateBloomSkill() {
        bloomSkill.active = true;
        bloomSkill.onCooldown = true;
        bloomSkill.startTime = GetTickCount();
        bloomSkill.duration = 0;  // 立即结束的技能
        bloomSkill.cooldownEnd = bloomSkill.startTime + SKILL_COOLDOWN_3;

        // 寻找主球位置
        int mainBallIndex = -1;
        for (int i = 0; i < MAX_BALLS; i++) {
            if (balls[i].active && !balls[i].isSplit) {
                mainBallIndex = i;
                break;
            }
        }

        // 如果找不到主球，使用第一个活跃的球
        if (mainBallIndex == -1) {
            for (int i = 0; i < MAX_BALLS; i++) {
                if (balls[i].active) {
                    mainBallIndex = i;
                    break;
                }
            }
        }

        // 如果有活跃的球，发射刺
        if (mainBallIndex != -1) {
            Ball* mainBall = &balls[mainBallIndex];

            // 初始化8个方向的刺（360度均匀分布）
            const float PI = 3.14159265358979323846;
            for (int i = 0; i < MAX_SPIKES; i++) {
                float angle = i * (2 * PI / MAX_SPIKES);  // 每个刺间隔45度
                int index = i % MAX_SPIKES;

                spikes[index].x = mainBall->x;
                spikes[index].y = mainBall->y;
                spikes[index].length = 10;  // 刺的长度
                spikes[index].dx = (int)(5 * cos(angle));  // 水平方向速度
                spikes[index].dy = (int)(5 * sin(angle));  // 垂直方向速度
                spikes[index].color = COLOR_SPIKE;
                spikes[index].active = true;
            }
        }
    }

    // 更新技能状态
    void updateSkills() {
        DWORD currentTime = GetTickCount();

        // 检查倍化技能
        if (enlargeSkill.active && currentTime > enlargeSkill.startTime + enlargeSkill.duration) {
            enlargeSkill.active = false;
            // 恢复球的大小
            for (int i = 0; i < MAX_BALLS; i++) {
                if (balls[i].active) {
                    balls[i].radius = BALL_RADIUS;
                }
            }
        }

        if (enlargeSkill.onCooldown && currentTime > enlargeSkill.cooldownEnd) {
            enlargeSkill.onCooldown = false;
        }

        // 检查分裂技能
        if (splitSkill.active && currentTime > splitSkill.startTime + splitSkill.duration) {
            splitSkill.active = false;
            // 移除分裂球
            for (int i = 0; i < MAX_BALLS; i++) {
                if (balls[i].active && balls[i].isSplit) {
                    balls[i].active = false;
                    ballCount--;
                }
            }

            // 检查是否所有球都消失了
            bool allBallsLost = true;
            for (int i = 0; i < MAX_BALLS; i++) {
                if (balls[i].active) {
                    allBallsLost = false;
                    break;
                }
            }

            if (allBallsLost) {
                resetBall();
            }
        }

        if (splitSkill.onCooldown && currentTime > splitSkill.cooldownEnd) {
            splitSkill.onCooldown = false;
        }

        // 检查绽放技能
        if (bloomSkill.active) {
            // 绽放技能为瞬时技能，激活后立即结束
            bloomSkill.active = false;
        }

        if (bloomSkill.onCooldown && currentTime > bloomSkill.cooldownEnd) {
            bloomSkill.onCooldown = false;
        }
    }

    // 重置球的位置
    void resetBall() {
        initBall();
        gameState = GAME_PAUSED;
    }

    // 游戏结束
    void gameOver() {
        gameState = GAME_OVER;
    }

    // 游戏胜利
    void gameWon() {
        gameState = GAME_WON;
    }

    // 主函数
    int main() {
        initGame();

        // 游戏循环
        while (true) {
            handleInput();
            updateGame();
            drawGame();
            Sleep(10);  // 控制游戏速度
        }

        EndBatchDraw();  // 关闭双缓冲
        closegraph();    // 关闭图形界面
        return 0;
    }
