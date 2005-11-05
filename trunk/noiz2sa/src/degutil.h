/*
 * $Id$
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Degutil header file.
 *
 * @version $Revision$
 */
#define DIV 1024
#define TAN_TABLE_SIZE 1024
#define SC_TABLE_SIZE 1024

extern int sctbl[];

void initDegutil();
int getDeg(int x, int y);
int getDistance(int x, int y);
