/*
 * $Id$
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Barrage data.
 *
 * @version $Revision$
 */
#ifndef BARRAGEMANAGER_H_
#define BARRAGEMANAGER_H_

#include "bulletml/bulletmlparser.h"
#include "bulletml/bulletmlparser-tinyxml.h"
#include "bulletml/bulletmlrunner.h"

#define BARRAGE_TYPE_NUM 3
#define BARRAGE_MAX 16

#define BOSS_TYPE 3

typedef struct {
  BulletMLParser *bulletml;
  float maxRank, rank;
  int type;
  int frq;
} Barrage;

extern "C" {
#include "brgmng_mtd.h"
}
#endif
