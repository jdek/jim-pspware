#ifndef _LIB_PSPMC_H_
#define _LIB_PSPMC_H_

/**
 *  main routine 
 */
int libpspmc_init();
int libpspmc_end();
int libpspmc_stop();
int libpspmc_loadplay(char *filepath);
int libpspmc_geteos();
int libpspmc_getinfo(unsigned char *string);
int libpspmc_gettime(unsigned char *string);

#endif
