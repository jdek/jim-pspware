/*
 * $Id$
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Player data.
 *
 * @version $Revision$
 */
#include "vector.h"

typedef struct {
  Vector pos;
  int cnt, shotCnt;
  int speed;
  int invCnt;
} Ship;

extern Ship ship;

void initShip();
void moveShip();
void drawShip();
void destroyShip();
int getPlayerDeg(int x, int y);
