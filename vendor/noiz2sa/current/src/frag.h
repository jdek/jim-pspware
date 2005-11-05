/*
 * $Id$
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Fragment data.
 *
 * @version $Revision$
 */
#include "vector.h"

typedef struct {
  Vector pos, vel;
  int width, height;
  int cnt;
  int spc;
} Frag;

#define FRAG_MAX 64

void initFrags();
void moveFrags();
void drawFrags();
void addShotFrag(Vector *pos);
void addEnemyFrag(Vector *p, int mx, int my, int type);
void addShipFrag(Vector *p);
void addClearFrag(Vector *p, Vector *v);
