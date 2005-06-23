#ifndef _CODEC_H_
#define _CODEC_H_

typedef enum {
    CODEC_MP3,
    CODEC_OGG,
    CODEC_MOD,
    MAX_CODECS
} codecIndex;

char validExtensions[MAX_CODECS][3];

typedef void (*fd_initFunc) (int);
typedef int (*fd_loadFunc) (char *);
typedef int (*fd_playFunc) (void);
typedef void (*fd_pauseFunc) (void);
typedef int (*fd_stopFunc) (void);
typedef void (*fd_endFunc) (void);
typedef void (*fd_tickFunc) (void);

typedef struct {
    fd_initFunc init;
    fd_loadFunc load;
    fd_playFunc play;
    fd_pauseFunc pause;
    fd_stopFunc stop;
    fd_endFunc end;
    fd_tickFunc tick;
} codecStubs;

#endif
