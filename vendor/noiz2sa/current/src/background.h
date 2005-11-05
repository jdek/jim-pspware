/*
 * $Id$
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Backgournd graphics data.
 *
 * @version $Revision$
 */
#include "vector.h"

typedef struct {
  int x, y, z;
  int width, height;
} Board;

#define BOARD_MAX 256

void initBackground();
void setStageBackground(int stage);
void moveBackground();
void drawBackground();
