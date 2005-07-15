// filer from RIN

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <psptypes.h>

// max 9
#define SAVE_SLOT_MAX		7

extern char LastPath[], FilerMsg[];

typedef struct {
	unsigned char  flag;
	char           date [32];
	int            size;
	unsigned char  compression; // 0 = None, 1 = RLE, 2+ = Reserved
	
	unsigned char  thumbflag;
	unsigned short thumbnail [128 * 112];
} SAVE_SLOT_INFO, *LPSAVE_SLOT_INFO;

extern LPSAVE_SLOT_INFO save_slots; //SAVE_SLOT_MAX + 1

int getExtId(const char *szFilePath);

int searchFile(const char *path, const char *name);
int getFilePath(char *out);

typedef struct {
	u8 *p_rom_image;			// pointer to rom image
	unsigned long rom_size;				// rom size
	char szFileName[_MAX_PATH + 1];	// extracted file name
}ROM_INFO, *LPROM_INFO;

int funcUnzipCallback(int nCallbackId, unsigned long ulExtractSize, unsigned long ulCurrentPosition,
                      const void *pData, unsigned long ulDataSize, unsigned long ulUserData);

void menu_frame(const unsigned char *msg0, const unsigned char *msg1);
void get_slotdate(const char *path);
void get_screenshot(unsigned char *buf);
void get_thumbs(const char *path);
void save_thumb(const char *path);
void delete_file(const char *path);

// —LŒø‚ÈŠg’£Žq
enum {
	EXT_SFC,
	EXT_SMC,
	EXT_ZIP,
	EXT_UNKNOWN
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

