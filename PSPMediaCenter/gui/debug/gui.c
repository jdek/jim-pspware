/*********************************************************************
* 
*  Debug based gui for PSP Media Center
*  adresd 2005
*/
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspaudio.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <string.h>
#include <pspmoduleinfo.h>
#include <pspaudiolib.h>

#include "../../codec.h"

//  These are the headers for the different codecs
//  auto-generated by the makefile
#include "../../codecincs.h"

/* Define printf, just to make typing easier */
#define printf  pspDebugScreenPrintf

// Common externs
extern unsigned char banner[];
extern codecStubs stubs[100];
extern codecStubs *decoder;
extern int errno, __errno;
extern int codecnum ;

static void strcat2(char *dest, char *src)
{
  int pos = 0;
  int pos2 = 0;
  while (*(dest + pos2) != 0)
    pos2++;
  while (*(src + pos) != 0) {
    *(dest + pos2) = *(src + pos);
    pos++;
    pos2++;
  }
  *(dest + pos2) = 0;
}

static int forceskip;
static void playmedia(char *rootpath, char *modname)
{
  char filename[200];
  u32 buttonsold;
  ctrl_data_t pad;
  int codec;
  int finished = 0;

  filename[0] = 0;
  strcat2(filename, rootpath);
  strcat2(filename, modname);

  pspDebugScreenClear();
  printf("%s\n\n", banner);
  printf("loading media File : %s\n", modname);

  //determine codec of the file
  for (codec = 0; codec <= codecnum; codec++)
    if (strncasecmp(&modname[strlen(modname) - 3], stubs[codec].extension, 3) == 0) {
      decoder = &stubs[codec];
      break;
    }

    decoder->init(0);
    if (decoder->load(filename)) {
      decoder->play();

      pspDebugScreenSetXY(0, 32);
      printf("X = Play.  O = Stop.  START = Tune Select.  SELECT = Exit.\n");
      pspDebugScreenSetXY(0, 26);
      printf("Playing\n\n");

      forceskip = 0;

      sceCtrlReadBufferPositive(&pad, 1);
      buttonsold = pad.buttons;
      while (finished == 0) {
        sceDisplayWaitVblankStart();
        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.buttons != buttonsold) {
          if (pad.buttons & CTRL_LTRIGGER) { // Previous tune
            forceskip = 1;
            finished = 1;
          }
          if (pad.buttons & CTRL_RTRIGGER) {  // Next tune
            forceskip = 2;
            finished = 1;
          }
          if (pad.buttons & CTRL_CIRCLE)
            decoder->stop();
          if (pad.buttons & CTRL_CROSS)
            decoder->pause();
          if (pad.buttons & CTRL_START)
            finished = 1;
          if (pad.buttons & CTRL_SELECT)
            finished = 2;
          buttonsold = pad.buttons;
        }
      }
      decoder->stop();
      decoder->end();
      if (finished == 2) {
        pspAudioEnd();
        sceKernelExitGame();
      }
    }
}

static SceIoDirent dirent;

static char *mods_infoname[1000];
static int mods_infonum;


static void sortmedialist()
{
  int found = 1;
  int swap;
  int count;
  char *temp;
  while (found == 1) {
    found = 0;
    for (count = 0; count < (mods_infonum - 1); count++) {
      swap = 0;
      if (mods_infoname[count][0] > mods_infoname[count + 1][0])
        swap = 1;
      else if ((mods_infoname[count][0] == mods_infoname[count + 1][0]) &&
        (mods_infoname[count][1] > mods_infoname[count + 1][1]))
        swap = 1;

      if (swap == 1) {	// Swap entries
        temp = mods_infoname[count];
        mods_infoname[count] = mods_infoname[count + 1];
        mods_infoname[count + 1] = temp;
        found = 1;
      }
    }
  }
}

static void fillmedialist(char *path)
{
  int dirid;
  int retval;
  char temp[4];
  int count = 0;
  int x;
  int found;
  if ((dirid = sceIoDopen(path)) > 0) {	//  Opened ok
    retval = 1;
    while ((retval > 0) && (count < 999)) {
      retval = sceIoDread(dirid, (SceIoDirent *) & dirent);
      if (retval > 0) {
        if (dirent.d_name[0] != 0) {
          // Only add files of types known to codecs loaded
          strcpy(temp, &dirent.d_name[strlen(dirent.d_name) - 3]);
          // Always check in lower, so conv to lower
          if ((temp[0] >= 'A') && (temp[0] <= 'Z'))
            temp[0] += 'a' - 'A';
          if ((temp[1] >= 'A') && (temp[1] <= 'Z'))
            temp[1] += 'a' - 'A';
          if ((temp[2] >= 'A') && (temp[2] <= 'Z'))
            temp[2] += 'a' - 'A';
          // Now check against the codecs known to us
          found = 0;
          for (x = 0; x < codecnum; x++)
            if (strncmp(temp, stubs[x].extension, 3) == 0)
              found = 1;
          if (found == 1) {
            mods_infoname[count] = (char *) malloc(200);
            memcpy(mods_infoname[count], dirent.d_name, 200);
            count++;
          }
        }
      }
    }
    sceIoDclose(dirid);
  }
  mods_infonum = count;
  sortmedialist();
}

static char *selectmedia()
{
  ctrl_data_t pad;
  static int highlight = 0;
  int highlightold;
  int finished = 0;
  int count;
  int x, y;
  u32 buttonsold = 0;
  int basepos;

  if (forceskip != 0) { // we are forcing a skip
    if (forceskip == 1) { // previous tune
      if (highlight != 0) highlight--;
    }
    else if (forceskip == 2) { // next tune
      if (highlight != mods_infonum) highlight++;
    }
    forceskip = 0;
    return mods_infoname[highlight];
  }

  printf("Select media to play:\n\n");
  // Save screen position
  x = pspDebugScreenGetX();
  y = pspDebugScreenGetY();

  sceCtrlReadBufferPositive(&pad, 1);
  buttonsold = pad.buttons;

  pspDebugScreenSetXY(0, 32);
  printf("Up/Down = Move cursor.  X = Select.  SELECT = Exit.\n");

  highlightold = -1;
  while (finished == 0) {	// Draw the menu firstly
    sceDisplayWaitVblankStart();
    if (highlightold != highlight) {
      // Calc position in the list, given number of files and highlight position
      if (highlight < 11)
        basepos = 0;
      else		//  we must scroll
        basepos = highlight - 11;
      pspDebugScreenSetXY(x, y);
      for (count = basepos; count < basepos + 22; count++) {
        if (count >= mods_infonum)
          printf("\n");
        else {
          if (highlight == count)
            printf("-> %02d - %-50s \n", count, mods_infoname[count]);
          else
            printf("   %02d - %-50s \n", count, mods_infoname[count]);
        }
      }
    }
    highlightold = highlight;
    // Now read the keys and act appropriately
    sceCtrlReadBufferPositive(&pad, 1);

    if (buttonsold != pad.buttons) {
      if (pad.buttons & CTRL_RIGHT)
        if (highlight < (mods_infonum - 11))
          highlight += 10;
        else
          highlight = mods_infonum-1;
      if (pad.buttons & CTRL_LEFT)
        if (highlight >= 10)
          highlight -= 10;
        else
          highlight = 0;
      if (pad.buttons & CTRL_UP)
        if (highlight >= 1)
          highlight--;
      if (pad.buttons & CTRL_DOWN)
        if (highlight < (mods_infonum - 1))
          highlight++;
      if (pad.buttons & CTRL_CROSS)
        return mods_infoname[highlight];
      if (pad.buttons & CTRL_SELECT)
        return 0;
    }
    buttonsold = pad.buttons;
  }
}

static void getproperpath(char *dest, char *src)
{
  int pos;
  int found = -1;
  pos = 0;
  while (*(src + pos) != 0) {
    if (*(src + pos) == '/')
      found = pos;
    *(dest + pos) = *(src + pos);
    pos++;
  }
  if (found != -1)
    *(dest + found + 1) = 0;
}

/* main routine */
int gui_main(void)
{
  char rootpath[200];
  char *modfile;
  int stubnum;

  pspDebugScreenInit();
  pspDebugScreenClear();
  sprintf(rootpath, "ms0:/PSP/MUSIC/");

  fillmedialist(rootpath);

  //  Loop around, offering a mod, till they cancel
  modfile = 1;
  forceskip = 0;
  while (modfile != 0) {
    // Setup screen  so it doesnt get messy
    pspDebugScreenClear();
    printf("%s\n\n", banner);

    {
      int c;
      printf("filetypes : ");
      for (c = 0; c < codecnum; c++)
        printf("%s ", stubs[c].extension);
      printf("\n\n");
    }
    // Filetype list
    printf("Media Path: %s\n\n", rootpath);

    modfile = selectmedia();
    if (modfile != 0) {
      playmedia(rootpath, modfile);
    }
  }
  return 0;
}