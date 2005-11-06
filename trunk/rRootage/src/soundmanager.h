/*
 * $Id$
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * BGM/SE manager header file.
 *
 * @version $Revision$
 */
void closeSound();
void initSound();
void playMusic(int idx);
void fadeMusic();
void stopMusic();
void playChunk(int idx);
void haltChunk(int idx);
