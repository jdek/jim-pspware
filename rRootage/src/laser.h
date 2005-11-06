/*
 * $Id$
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Players laser.
 *
 * @version $Revision$
 */
#include "vector.h"

typedef struct {
  int y;
  int color;
  int cnt;
} Laser;

void initLasers();
void moveLasers();
void drawLasers();
void addLaser();
