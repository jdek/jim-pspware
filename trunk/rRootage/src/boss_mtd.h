/*
 * $Id$
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Boss methods.
 *
 * @version $Revision$
 */
#include "vector.h"

void createBoss(int seed, double rank, int round);
void initBoss();
void moveBoss();
void drawBoss();
int checkHitDownside(int x);
int checkHitUpside();
void damageBoss(int dmg);
void damageBossLaser(int cnt);
void weakenBoss();
void drawBossState();
Vector* getBossPos();
