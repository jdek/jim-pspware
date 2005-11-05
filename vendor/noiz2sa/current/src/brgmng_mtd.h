/*
 * $Id$
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Stage data.
 *
 * @version $Revision$
 */
void initBarragemanager();
void closeBarragemanager();
void initBarrages(int seed, float startLevel, float li);
void setBarrages(float level, int bm, int midMode);
void addBullets();
void addBossBullet();
void bossDestroied();

extern int scene;
extern int endless, insane;
