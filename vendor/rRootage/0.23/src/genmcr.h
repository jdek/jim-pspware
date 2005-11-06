/*
 * $Id$
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * General macros.
 *
 * @version $Revision$
 */
#include "rand.h"

#define randN(N) ((int)(nextRand()>>5)%(N))
#define randNS(N) (((int)(nextRand()>>5))%(N<<1)-N)
#define randNS2(N) ((((int)(nextRand()>>5))%(N)-(N>>1)) + (((int)(nextRand()>>5))%(N)-(N>>1)))
#define absN(a) ((a) < 0 ? - (a) : (a))

#define NOT_EXIST -999999
