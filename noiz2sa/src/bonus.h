/*
 * $Id$
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Bonus item data.
 *
 * @version $Revision$
 */
#include "vector.h"

typedef struct {
  Vector pos, vel;
  int cnt;
  int down;
} Bonus;

#define BONUS_MAX 256

extern int bonusScore;

void resetBonusScore();
void initBonuses();
void moveBonuses();
void drawBonuses();
void addBonus(Vector *pos, Vector *vel);
