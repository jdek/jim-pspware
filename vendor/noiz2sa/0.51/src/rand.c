/*
 * $Id$
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Make random number.
 *
 * @version $Revision$
 */
#include "rand.h"

static unsigned int multiplier = 8513;
static unsigned int addend = 179;

unsigned int nextRandInt(unsigned int *v) {
  *v = *v * multiplier + addend;
  return *v;
}
