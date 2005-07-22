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
	unsigned char  compression; // 0 = None, 1 = RLE, 2 = GZ, 3+ = Reserved
	
	unsigned char  thumbflag;
	unsigned short thumbnail [128 * 112];
} SAVE_SLOT_INFO, *LPSAVE_SLOT_INFO;

extern LPSAVE_SLOT_INFO save_slots; //SAVE_SLOT_MAX + 1

// Extension Bitmasks - No file will ever have more than one extension,
//                      this is merely so you can specify arbitrary
//                      combinations of files.
enum {
	EXT_SFC     = 0x01,
	EXT_SMC     = 0x02,
	EXT_ZIP     = 0x04,
	EXT_CFG     = 0x08,
	EXT_STATE   = 0x10,
	EXT_THUMB   = 0x20,
	EXT_SRAM    = 0x40,
	EXT_NONE    = 0x40000000, // IS a file, but has no extension
	EXT_UNKNOWN = 0x80000000, // IS a file, but extension unknown
	EXT_DIR     = 0x00,       // Directory, extension undefined
};

#define EXT_MASK_ROM        ( EXT_SFC  |  EXT_SMC  | EXT_ZIP)
#define EXT_MASK_STATE_SAVE (EXT_STATE | EXT_THUMB)
#define EXT_MASK_RESOURCES  (EXT_STATE | EXT_THUMB | EXT_CFG | EXT_SRAM)

int getExtId(const char *szFilePath);

int searchFile(const char *path, const char *name);
int getFilePath(char *out, int ext_mask);

typedef struct {
	uint8         *p_rom_image;                // pointer to rom image
	unsigned long  rom_size;                   // rom size
	char           szFileName [_MAX_PATH + 1]; // extracted file name
}ROM_INFO, *LPROM_INFO;

int funcUnzipCallback(int nCallbackId, unsigned long ulExtractSize, unsigned long ulCurrentPosition,
                      const void *pData, unsigned long ulDataSize, unsigned long ulUserData);

void  menu_frame(const unsigned char *msg0, const unsigned char *msg1);
void  get_slotdate(const char *path);
void  get_screenshot(unsigned char *buf);
void  get_thumbs(const char *path);
void  save_thumb(const char *path);

bool8 delete_file_confirm (const char *path, const char *name);
void  delete_file         (const char *path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

