/*
 * $Id$
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Foe methods.
 *
 * @version $Revision$
 */
#define FOE_ENEMY_POS_RATIO 1024

#define DEFAULT_SPEED_DOWN_BULLETS_NUM 100
#define EASY_SPEED_DOWN_BULLETS_NUM 80
#define HARD_SPEED_DOWN_BULLETS_NUM 120

extern int processSpeedDownBulletsNum;
extern int nowait;

void initFoes();
void closeFoes();
void moveFoes();
void clearFoes();
void clearFoesZako();
void drawBulletsWake();
void drawFoes();
void drawBullets();
