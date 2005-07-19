#ifndef __PROFILER_H__
#define __PROFILER_H__

#include <sys/time.h>

#ifdef PROFILE_GFX
struct gfx_profile_t {
  long time_RenderScreen;
  long time_DrawOBJS;
  long time_SelectTileRenderer;
  long time_DrawBackground;
  long time_DrawBackground_8;
  long time_DrawBackground_16;
  long time_DrawBackgroundMosaic;
  long time_DrawBackgroundOffset;
  long time_DrawBackgroundMode5;
};

extern gfx_profile_t gfx_profile;

#define START_PROFILE_GFX_FUNC(name)     \
  static timeval __func_time_start;      \
  static timeval __func_time_finish;     \
                                         \
  gettimeofday (& __func_time_start, 0);

#define FINISH_PROFILE_GFX_FUNC(name)         \
  gettimeofday (& __func_time_finish, 0);     \
                                              \
  gfx_profile.time_##name += (                \
    (((__func_time_finish.tv_sec -            \
       __func_time_start.tv_sec) * 1000000) + \
     (__func_time_finish.tv_usec) -           \
      __func_time_start.tv_usec)              \
  );
#else
#define START_PROFILE_GFX_FUNC(name)  ;
#define FINISH_PROFILE_GFX_FUNC(name) ;
#endif /* PROFILE_GFX */


#ifdef PROFILE_TILES
struct tile_profile_t {
  long time_DrawTile;
  long time_DrawTilex2;
  long time_DrawTilex2x2;
  long time_DrawClippedTilex2x2;
  long time_DrawTile16;
  long time_DrawTile16_OBJ;
  long time_DrawClippedTile16;
  long time_DrawTile16x2;
  long time_DrawClippedTile16x2;
  long time_DrawTile16x2x2;
  long time_DrawClippedTile16x2x2;
  long time_DrawTile16Add;
  long time_DrawClippedTile16Add;
  long time_DrawTile16Add1_2;
  long time_DrawClippedTile16Add1_2;
  long time_DrawTile16Sub;
  long time_DrawClippedTile16Sub;
  long time_DrawTile16sub1_2;
  long time_DrawClippedTile16Sub1_2;
  long time_DrawTile16FixedAdd1_2;
  long time_DrawClippedTile16FixAdd1_2;
  long time_DrawTile16FixedSub1_2;
  long time_DrawClippedTile16FixedSub1_2;
  long time_DrawLargePixel;
  long time_DrawLargePixel16;
  long time_DrawLargePixel16Add;
  long time_DrawLargePixel16Add1_2;
  long time_DrawLargePixel16Sub;
  long time_DrawLargePixel16Sub1_2;

  long calls_DrawTile16;
  long calls_DrawTile16_OBJ;
  long calls_DrawClippedTile16;
};

extern gfx_profile_t gfx_profile;

#define START_PROFILE_TILE_FUNC(name)     \
  static timeval __func_time_start;       \
  static timeval __func_time_finish;      \
                                          \
  gettimeofday (& __func_time_start, 0);

#define FINISH_PROFILE_TILE_FUNC(name)        \
  gettimeofday (& __func_time_finish, 0);     \
                                              \
  tile_profile.time_##name += (               \
    (((__func_time_finish.tv_sec -            \
       __func_time_start.tv_sec) * 1000000) + \
     (__func_time_finish.tv_usec) -           \
      __func_time_start.tv_usec)              \
  );
#else
#define START_PROFILE_TILE_FUNC(name)  ;
#define FINISH_PROFILE_TILE_FUNC(name) ;
#endif /* PROFILE_TILES */

#endif /* __PROFILER_H__ */
