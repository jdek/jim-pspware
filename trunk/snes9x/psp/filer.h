// filer from RIN

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <psptypes.h>

// max 9
#define SAVE_SLOT_MAX		4

extern char LastPath[], FilerMsg[];
extern unsigned char slotflag[];
extern char slotdate[SAVE_SLOT_MAX+1][32];
extern unsigned char thumbflag[SAVE_SLOT_MAX+1];
extern unsigned short slot_thumb[SAVE_SLOT_MAX+1][128 * 112];

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

void menu_frame(const char *msg0, const char *msg1);
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

