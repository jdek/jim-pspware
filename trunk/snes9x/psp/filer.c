#include "psp.h"
#include "pg.h"
#include "filer.h"
#include "psp/zlibInterface.h"

#include "port.h"

#include <stdlib.h>
#include <stdio.h>

#define true 1
#define false 0

extern u32 new_pad;

struct SceIoDirent files[MAX_ENTRY];
int nfiles;

void menu_frame(const unsigned char *msg0, const unsigned char *msg1)
{
	long BatteryVolt_ret        = scePowerGetBatteryVolt        (); // Voltage: 5000 = 5.000 Volts
	long BatteryCharging_ret    = scePowerIsBatteryCharging     (); // 
	int  BatteryLifePercent_ret = scePowerGetBatteryLifePercent (); // 
	int  BatteryLifeTime_ret    = scePowerGetBatteryLifeTime    (); // Estimated number of minutes the battery will last
	
	char message               [256];
	char BatteryVolt_ret_text  [16];
	char BatteryCharge_percent [16];
	
	bool8 BatteryFull = FALSE;

	pgFillvram (0x9063);
	mh_print   (289, 0, (unsigned char *)" ■ uo_Snes9x for PSP Ver0.02pd2 ■", RGB (85,85,95));
	
	if ((BatteryVolt_ret > 999) && (BatteryVolt_ret < 9999)) {
		sprintf (BatteryVolt_ret_text, "(%1.3fV)", (float)BatteryVolt_ret / 1000);
	} else {
		strcpy (BatteryVolt_ret_text, "(No Battery or Bad Voltage)");
	}
	
	if (BatteryLifePercent_ret < 100) {
		sprintf (BatteryCharge_percent, "%d%%", BatteryLifePercent_ret);
	} else {
		BatteryFull = TRUE;
		strcpy (BatteryCharge_percent, "Full");
	}
	
	// Running on AC power...
	if (BatteryCharging_ret <  0) {
		mh_print (0, 0, (unsigned char*)"[Running on AC Power]", RGB(0,255,0));
	}
	
	// Battery is being used...
	else if (BatteryCharging_ret == 0) {
		if (BatteryLifeTime_ret > 0 && BatteryLifeTime_ret < 330 /* 5:30 */) {
			sprintf (message, "[Battery: %s %s -:- %01d:%02d]", BatteryCharge_percent, BatteryVolt_ret_text,
		                                                            BatteryLifeTime_ret / 60, BatteryLifeTime_ret % 60);
		} else {
			sprintf (message, "[Battery: %s %s]", BatteryCharge_percent, BatteryVolt_ret_text);
		}
		mh_print (0, 0, (unsigned char*)message, BatteryFull ? RGB (0,0,255) :
		                                                        RGB (255,0,0));
	}
	
	// Battery is being charged...
	else {
		long BatteryTemp_ret = scePowerGetBatteryTemp ();
		// If the battery temp. is > 38C (100F), display the temp.
		if ((BatteryTemp_ret > 38) && (BatteryTemp_ret < 100)) {
			sprintf (message, "[Charging: %s %s - %dｰ F]", BatteryCharge_percent, BatteryVolt_ret_text,
					                               (int)((9.0f/5.0f) * (float)BatteryTemp_ret) + 32);
			mh_print (0, 0, (unsigned char*)message, RGB (255,0,0));
		} else {
			sprintf  (message, "[Charging: %s %s]", BatteryCharge_percent, BatteryVolt_ret_text);
			mh_print (0, 0, (unsigned char*)message, RGB (0,0,255));
		}
	}

	// メッセージなど
	if(msg0!=0) mh_print(17, 14, msg0, RGB(105,105,115));
	pgDrawFrame(17,25,463,248,RGB(85,85,95));
	pgDrawFrame(18,26,462,247,RGB(85,85,95));
	// 操作説明
	if(msg1!=0) mh_print(17, 252, msg1, RGB(105,105,115));
}

void SJISCopy(struct SceIoDirent *a, unsigned char *file)
{
	unsigned char ca;
	int i;

	for(i=0;i<=strlen(a->d_name);i++){
		ca = a->d_name[i];
		if (((0x81 <= ca)&&(ca <= 0x9f))
		|| ((0xe0 <= ca)&&(ca <= 0xef))){
			file[i++] = ca;
			file[i] = a->d_name[i];
		}
		else{
			if(ca>='a' && ca<='z') ca-=0x20;
			file[i] = ca;
		}
	}

}
int cmpFile(struct SceIoDirent *a, struct SceIoDirent *b)
{
    char file1[0x108];
    char file2[0x108];
	unsigned char ca, cb;
	int i, n, ret;

	if(a->d_stat.st_attr==b->d_stat.st_attr){
		SJISCopy(a, (unsigned char *)file1);
		SJISCopy(b, (unsigned char *)file2);
		n=strlen(file1);
		for(i=0; i<=n; i++){
			ca=file1[i]; cb=file2[i];
			ret = ca-cb;
			if(ret!=0) return ret;
		}
		return 0;
	}
	
	if(a->d_stat.st_attr & FIO_SO_IFDIR)	return -1;
	else					return 1;
}

void sort(struct SceIoDirent *a, int left, int right) {
	struct SceIoDirent tmp, pivot;
	int i, p;
	
	if (left < right) {
		pivot = a[left];
		p = left;
		for (i=left+1; i<=right; i++) {
			if (cmpFile(&a[i],&pivot)<0){
				p=p+1;
				tmp=a[p];
				a[p]=a[i];
				a[i]=tmp;
			}
		}
		a[left] = a[p];
		a[p] = pivot;
		sort(a, left, p-1);
		sort(a, p+1, right);
	}
}

// Map for extension strings and bitmasks
const struct {
	char *szExt;
	int   nExtId;
} stExtensions [] = {
	{ "sfc", EXT_SFC     },
	{ "smc", EXT_SMC     },
	{ "zip", EXT_ZIP     },
	{ "cfg", EXT_CFG     },
	{ "sv0", EXT_STATE   }, { "sv1", EXT_STATE }, { "sv2", EXT_STATE },
	{ "sv3", EXT_STATE   }, { "sv4", EXT_STATE }, { "sv5", EXT_STATE },
	{ "sv6", EXT_STATE   }, { "sv7", EXT_STATE },
	{ "tn0", EXT_THUMB   }, { "tn1", EXT_THUMB }, { "tn2", EXT_THUMB },
	{ "tn3", EXT_THUMB   }, { "tn4", EXT_THUMB }, { "tn5", EXT_THUMB },
	{ "tn6", EXT_THUMB   }, { "tn7", EXT_THUMB },
	{ "srm", EXT_SRAM    },
	{   0,   EXT_UNKNOWN }
};

int getExtId (const char *szFilePath)
{
	char *pszExt;
	int   i;
	
	// .. is a special case. It's a directory, but it doesn't have a trailing /
	if (! strcmp (szFilePath, ".."))
		return EXT_DIR;
	
	if ((pszExt = (char *)strrchr (szFilePath, '.'))) {
		pszExt++;
		
		for (i = 0; stExtensions [i].nExtId != EXT_UNKNOWN; i++) {
			if (! strcasecmp (stExtensions [i].szExt, pszExt)) {
				return stExtensions [i].nExtId;
			}
		}
	} else {
		// Missing a dot, this could be either a directory
		// or a file with no extension.

		// If it has a / it's a directory, and extensions
		// for directories are undefined.
		if (strrchr (szFilePath, '/'))
			return EXT_DIR;
		
		// File with no extension
		return EXT_NONE;
	}
	
	return EXT_UNKNOWN;
}

void getDir(const char *path, const int ext_mask) {
	int fd, b=0;
	
	nfiles = 0;
	memset (files, 0, sizeof(files));
	
	if(strcmp(path,"ms0:/")){
		strcpy(files[nfiles].d_name,"..");
		files[nfiles].d_stat.st_attr = FIO_SO_IFDIR;
		nfiles++;
		b=1;
	}
	
	fd = sceIoDopen(path);
	while(nfiles<MAX_ENTRY){
		if(sceIoDread(fd, &files[nfiles])<=0) break;
		if(files[nfiles].d_name[0] == '.') continue;
		if(files[nfiles].d_stat.st_attr == FIO_SO_IFDIR){
			strcat(files[nfiles].d_name, "/");
			nfiles++;
			continue;
		}

		if (ext_mask & getExtId (files [nfiles].d_name))
			nfiles++;
	}

	sceIoDclose(fd);
	if(b)
		sort(files+1, 0, nfiles-2);
	else
		sort(files, 0, nfiles-1);
}

char LastPath [_MAX_PATH];
char FilerMsg [256];
static int  dialog_y;

int getFilePath(char *out, int ext_mask)
{
	int original_ext_mask = ext_mask;
	
	unsigned long color;
	static int sel=0;
	int top, rows=21, x, y, h, i, bMsg=0, up=0;
	char path[_MAX_PATH], oldDir[_MAX_PATH], *p;

	top = sel-3;
	
	strcpy(path, LastPath);
	if(FilerMsg[0])
		bMsg=1;

	getDir (path, ext_mask);

	for(;;){
		readpad ();

		if(new_pad)
			bMsg=0;
		if(new_pad & PSP_CTRL_CIRCLE){
			if(files[sel].d_stat.st_attr == FIO_SO_IFDIR){
				if(!strcmp(files[sel].d_name,"..")){
					up=1;
				}else{
					strcat(path,files[sel].d_name);
					getDir(path, ext_mask);
					sel=0;
				}
			}else{
				if (original_ext_mask != 0xffffffff) {
					strcpy(out, path);
					strcat(out, files[sel].d_name);
					strcpy(LastPath,path);
					return 1;
				} else {
					return 1;
				}
			}
		}else if(new_pad & PSP_CTRL_CROSS){
			return 0;
		}else if(new_pad & PSP_CTRL_TRIANGLE){
			up=1;
		}else if(new_pad & PSP_CTRL_UP){
			sel--;
		}else if(new_pad & PSP_CTRL_DOWN){
			sel++;
		}else if(new_pad & PSP_CTRL_LEFT){
			sel-=10;
		}else if(new_pad & PSP_CTRL_RIGHT){
			sel+=10;
		}else if(new_pad & PSP_CTRL_LTRIGGER){
				sel-=20;
//			sel-=21;
//			top=sel-20;
 		}else if(new_pad & PSP_CTRL_RTRIGGER){
				sel+=20;
//			sel+=21;
//			top=sel;
		}

		else if (new_pad & PSP_CTRL_START) {
			if (original_ext_mask == 0xffffffff) {
				switch (ext_mask) {
					case EXT_MASK_ROM:
						ext_mask = EXT_MASK_STATE_SAVE;
						break;
					case EXT_MASK_STATE_SAVE:
						ext_mask = EXT_MASK_RESOURCES;
						break;
					case EXT_MASK_RESOURCES:
						ext_mask = 0xffffffff;
						break;
					default:
						ext_mask = EXT_MASK_ROM;
						break;
				}

				getDir (path, ext_mask);
			}
		}

		// Delete the selected file (confirm first)
		else if (new_pad & PSP_CTRL_SQUARE) {
			if (files [sel].d_stat.st_attr != FIO_SO_IFDIR) {
				if (strcmp (files [sel].d_name, ".." )) {
					if (delete_file_confirm (path, files [sel].d_name))
						getDir (path, ext_mask);
				}
			}
		}
		
		if(up){
			if(strcmp(path,"ms0:/")){
				p=(char *)strrchr(path,'/');
				*p=0;
				p=(char *)strrchr(path,'/');
				p++;
				strcpy(oldDir,p);
				strcat(oldDir,"/");
				*p=0;
				getDir(path, ext_mask);
				sel=0;
				for(i=0; i<nfiles; i++) {
					if(!strcmp(oldDir, files[i].d_name)) {
						sel=i;
						top=sel-3;
						break;
					}
				}
			}
			up=0;
		}
		
		if(top > nfiles-rows)	top=nfiles-rows;
		if(top < 0)				top=0;
		if(sel >= nfiles)		sel=nfiles-1;
		if(sel < 0)				sel=0;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;
		
		const  unsigned char* frame_top;
		static unsigned char  frame_bottom [128];
		
		if (bMsg)
			frame_top = (unsigned char *)FilerMsg;
		else
			frame_top = (unsigned char *)path;

		// Operations that apply no matter what type of entry is selected
		if (original_ext_mask == 0xffffffff) {
			// 0xffffffff indicates this is file manager mode, no loading is possible
			strcpy ((char *)frame_bottom, "○ ／ ×：CLOSE　△：UP");
		} else {
			strcpy ((char *)frame_bottom, "○：OK　×：CANCEL　△：UP");
		}

		// If the selected entry is a file, add DELETE and COPY/MOVE
		if ((! (files [sel].d_stat.st_attr & FIO_SO_IFDIR)) &&
		    (strcmp (files [sel].d_name, ".."))) {
			strcat ((char *)frame_bottom, "　□：DELETE　SELECT：COPY/MOVE");
		}

		menu_frame (frame_top,frame_bottom);

		int total_size  = 0;
		int total_files = 0; // This is the number of files that match
		                     // the current filter (extension mask).
		
		// スクロールバー
		if(nfiles > rows){
			h = 219;
			pgDrawFrame(445,25,446,248,RGB(85,85,95));
			pgFillBox(448, h*top/nfiles + 27,
				460, h*(top+rows)/nfiles + 27,RGB(85,85,95));
		}

		// Calculate the total file size and number of files (minus directories)
		for (i=0; i<nfiles; i++) {
			if (ext_mask & getExtId (files [i].d_name)) {
				total_size += files [i].d_stat.st_size;
				total_files ++;
			}
		}

		x=28; y=32;
		for(i=0; i<rows; i++){
			if(top+i >= nfiles)
				break;

			// Shorthand
			const SceIoDirent* file = &files [top + i];

			if(top+i == sel) {
				color = RGB( 255,  0,   0 );
				dialog_y = y;
			}else {
				color = 0xffff;
			}

			// TODO: Cache this stuff
			const int filename_len = strlen (file->d_name);
			char*     szName       = file->d_name;

			// Truncate long filenames so that they don't overlap the file size.
			if (filename_len > 74) {
				static char szTruncatedName [128];
				strncpy (szTruncatedName, file->d_name, 74);

				// Add ... to denote truncation...
				strcpy ((szTruncatedName + 70), "...");

				szName = szTruncatedName;
			}

			mh_print (x, y, (unsigned char *)szName, color);

			// Dim the filesize info so that it's not painful to look at.
			if ((top + i) == sel)
							color = RGB (192, 0, 0);
			else
							color = RGB (192, 192, 192);

			// For non-directory entires, list the file size.
			if (! (file->d_stat.st_attr & FIO_SO_IFDIR)) {
				static char szSize [32];
				const  int  iSize = file->d_stat.st_size;

				if (iSize >= (1 << 20)) {
					sprintf (szSize, "%4.2f Mb",
						(float)((float)file->d_stat.st_size /
										(1024.0f * 1024.0f)));
				} else if (iSize >= (1 << 10)) {
					sprintf (szSize, "%#4i Kb",
						file->d_stat.st_size / 1024);
				} else {
					sprintf (szSize, "%#4i  b", file->d_stat.st_size);
				}

				// mh_print's coordinates are in pixels, not logical character coords.
				mh_print (402, y, (unsigned char *)szSize, color);
			}

			y += 10;
		}

		static char szTotalSize [32];
		if (total_size >= (1 << 20) || total_size == 0) {
			sprintf (szTotalSize, "Total Size: %6.2f Mb",
				(float)((float)total_size /
					(1024.0f * 1024.0f)));
		} else if (total_size >= (1 << 10)) {
			sprintf (szTotalSize, "Total Size: %#6i Kb",
				total_size / 1024);
		} else if (total_size > 0) {
			sprintf (szTotalSize, "Total Size: %#6i  b", total_size);
		}

		static char szTotalFiles [32];
		sprintf (szTotalFiles, "     Files: %#6i", total_files);

		mh_print (332, 252, (unsigned char *)szTotalSize,  RGB (85,85,95));
		mh_print (332, 262, (unsigned char *)szTotalFiles, RGB (85,85,95));

		if (original_ext_mask == 0xffffffff) {
			static char szFilterName [64];

			strcpy (szFilterName, "File Filter: ");
			
			switch (ext_mask) {
				case EXT_MASK_ROM:
					strcat (szFilterName, "      ROM Images");
					break;
				case EXT_MASK_STATE_SAVE:
					strcat (szFilterName, "     State Saves");
					break;
				case EXT_MASK_RESOURCES:
					strcat (szFilterName, "Snes9x Resources");
					break;
				case 0xffffffff:
					strcat (szFilterName, "       All Files");
					break;
				default:
					strcat (szFilterName, "         Unknown");
					break;
			}
			mh_print (300, 14, (unsigned char *)szFilterName, RGB (155,155,165));

			mh_print (17, 262, (unsigned char *)"Press START to change the File Filter", RGB (155,155,165));
		}

		pgScreenFlipV ();
	}
}

// せっかくなのでプログレスでも出してみます
void draw_load_rom_progress(unsigned long ulExtractSize, unsigned long ulCurrentPosition)
{
	int nPer = 100 * ulExtractSize / ulCurrentPosition;
	static int nOldPer = 0;
	if (nOldPer == (nPer & 0xFFFFFFFE)) {
		return ;
	}
	nOldPer = nPer;
	pgFillvram(0x9063);
	pgDrawFrame(88,121,392,141,RGB(85,85,95));
	pgFillBox(90,123, 90+nPer*3, 139,RGB(85,85,195));
	// ％
	char szPer[16];
	sprintf (szPer, "%i%%", nPer);
	pgPrint(28,16,0xffff,szPer);
	// pgScreenFlipV()を使うとpgWaitVが呼ばれてしまうのでこちらで。
	// プログレスだからちらついても良いよね～
	pgScreenFlip ();
}

// Unzip コールバック
int funcUnzipCallback(int nCallbackId, unsigned long ulExtractSize, unsigned long ulCurrentPosition,
                      const void *pData, unsigned long ulDataSize, unsigned long ulUserData)
{
    const char *pszFileName;
    int nExtId;
    const unsigned char *pbData;
    LPROM_INFO pRomInfo = (LPROM_INFO)ulUserData;

    switch(nCallbackId) {
    case UZCB_FIND_FILE:
		pszFileName = (const char *)pData;
		nExtId = getExtId(pszFileName);
		// 拡張子がSFCかSMCなら展開
		if (nExtId == EXT_SFC || nExtId == EXT_SMC) {
			// 展開する名前、rom sizeを覚えておく
			strcpy(pRomInfo->szFileName, pszFileName);
			pRomInfo->rom_size = ulExtractSize;
			return UZCBR_OK;
		}
        break;
    case UZCB_EXTRACT_PROGRESS:
		pbData = (const unsigned char *)pData;
		// 展開されたデータを格納しよう
		memcpy(pRomInfo->p_rom_image + ulCurrentPosition, pbData, ulDataSize);
		draw_load_rom_progress(ulCurrentPosition + ulDataSize, ulExtractSize);
		return UZCBR_OK;
        break;
    default: // unknown...
		pgFillvram(RGB(255,0,0));
		pgScreenFlipV();
        break;
    }
    return UZCBR_PASS;
}

extern void ustoa(unsigned short val, char *s);
extern bool8 S9xIsFreezeGameRLE (void* data);

SAVE_SLOT_INFO   _save_slots [SAVE_SLOT_MAX + 1];
LPSAVE_SLOT_INFO  save_slots = _save_slots;

void get_slotdate( const char *path )
{
	char name[_MAX_PATH + 1], tmp[8],*p;
	int i,j,fd;
	
	p = (char *)strrchr(path,'/') + 1;
	strcpy(name, p);
	*p = 0;
	
	nfiles = 0;
	memset (files, 0, sizeof(files));
	
	fd = sceIoDopen(path);
	while(nfiles<MAX_ENTRY){
		if(sceIoDread(fd, &files[nfiles])<=0) {break;}
		nfiles++;
	}
	sceIoDclose(fd);
	
	for(i=0; i<=SAVE_SLOT_MAX; i++){
		char* slotdate = save_slots [i].date;
		
		strcpy(slotdate,".sv0 - ");
		slotdate[3] = name[strlen(name)-1] = i + '0';
		for(j=0; j<nfiles; j++){
			if(!strcasecmp(name,files[j].d_name)){
				save_slots [i].size = files [j].d_stat.st_size;

				ustoa(files[j].d_stat.st_mtime.year,tmp);
				strcat(slotdate,tmp);
				strcat(slotdate,"/");
				
				if(files[j].d_stat.st_mtime.month < 10) strcat(slotdate,"0");
				ustoa(files[j].d_stat.st_mtime.month,tmp);
				strcat(slotdate,tmp);
				strcat(slotdate,"/");
				
				if(files[j].d_stat.st_mtime.day < 10) strcat(slotdate,"0");
				ustoa(files[j].d_stat.st_mtime.day,tmp);
				strcat(slotdate,tmp);
				strcat(slotdate," ");
				
				if(files[j].d_stat.st_mtime.hour < 10) strcat(slotdate,"0");
				ustoa(files[j].d_stat.st_mtime.hour,tmp);
				strcat(slotdate,tmp);
				strcat(slotdate,":");
				
				if(files[j].d_stat.st_mtime.minute < 10) strcat(slotdate,"0");
				ustoa(files[j].d_stat.st_mtime.minute,tmp);
				strcat(slotdate,tmp);
				strcat(slotdate,":");
				
				if(files[j].d_stat.st_mtime.second < 10) strcat(slotdate,"0");
				ustoa(files[j].d_stat.st_mtime.second,tmp);
				strcat(slotdate,tmp);
				
				save_slots[i].flag = true;

				save_slots [i].compression = 0;
				
				// Determine if the freeze file is compressed...
				if (save_slots [i].size > 5) {
					char filename [_MAX_PATH + 1];

					strcpy (filename, path);
					strcat (filename, name);

					FILE* freeze = fopen (filename, "r");

					if (freeze) {
						char header [5];
						fread  (header, 5, 1, freeze);
						fclose (freeze);

						if (S9xIsFreezeGameRLE ((void *)header))
							save_slots [i].compression = 1;
					}
				}
				break;
			}
		}
		if(j>=nfiles){
			strcat(save_slots[i].date,"not exist");
			save_slots[i].flag        = false;
			save_slots[i].compression = 0;
			save_slots[i].size        = 0;
		}
	}
}

unsigned short now_thumb[128 * 112];
void get_screenshot(unsigned char *buf)
{
	unsigned char *vptr0;
	unsigned short *rgbptr;
	unsigned short rgb, rgb2;
	int x,y,i;
	i = 0;

	const int y_incr = (PSP_Settings.bUseGUBlit ? (PSP_Settings.bSupportHiRes ? 2 : 1) : 2);
	
	vptr0=buf;
	for (y=0; y<224; y += y_incr) {
		rgbptr=(unsigned short *)vptr0;
		for (x=0; x<256; x+=2) {
			rgb = (*rgbptr & *(rgbptr+1)) + (((*rgbptr ^ *(rgbptr+1)) & 0x7bde) >> 1);
			rgb2 = (*(rgbptr+LINESIZE) & *(rgbptr+LINESIZE+1)) + (((*(rgbptr+LINESIZE) ^ *(rgbptr+LINESIZE+1)) & 0x7bde) >> 1);
			now_thumb[i++] = (rgb & rgb2) + (((rgb ^ rgb2) & 0x7bde) >> 1);
			rgbptr+=2;
		}
		vptr0+=LINESIZE*y_incr*2;
	}
}

void get_thumbs(const char *path)
{
	char name[_MAX_PATH + 1], filepath[_MAX_PATH + 1], tmp[8],*p;
	int i,j,fd;
	
	p = (char *)strrchr(path,'/') + 1;
	strcpy(name, p);
	*p = 0;
	
	nfiles = 0;
	memset (files, 0, sizeof(files));
	fd = sceIoDopen(path);
	while(nfiles<MAX_ENTRY){
		if(sceIoDread(fd, &files[nfiles])<=0) {break;}
		nfiles++;
	}
	sceIoDclose(fd);
	
	for(i=0; i<=SAVE_SLOT_MAX; i++){
		name[strlen(name)-1] = i + '0';
		for(j=0; j<nfiles; j++){
			if(!stricmp(name,files[j].d_name)){
				strcpy(filepath, path);
				strcat(filepath, name);
				fd = sceIoOpen( filepath, PSP_O_RDONLY, 0777 );
				if (fd>=0) {
					sceIoRead(fd, &save_slots[i].thumbnail, sizeof(save_slots[i].thumbnail));
					sceIoClose(fd);
				}
				save_slots[i].thumbflag = true;
				break;
			}
		}
		if(j>=nfiles){
			memset (save_slots[i].thumbnail, 0, sizeof(save_slots[i].thumbnail));
			save_slots[i].thumbflag = false;
		}
	}
}

void save_thumb(const char *path)
{
	int fd, size = 0;
	fd = sceIoOpen( path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777 );
	if (fd>=0) {
		size = sceIoWrite(fd, &now_thumb, sizeof(now_thumb));
		sceIoClose(fd);
	}
	if (size == sizeof(now_thumb)) return;
	sceIoRemove(path);
}

bool8 delete_file_confirm (const char *path, const char *name)
{
	//   ループカウント
	int  loop_cnt;
	//   ROM削除用パス
	char delete_path     [_MAX_PATH];
	//   タイトル文字列
	char title_string    [D_text_MAX];
	//   ダイアログテキストの保存用変数
	char dialog_text_all [D_text_all_MAX];
	
	// ダイアログタイトル文字列作成
	strcpy (title_string,    "       　【　　File Delete　　】　  \0");
	
	// メッセージ作成
	strcpy (dialog_text_all, name                                            );
	strcat (dialog_text_all, "\n\n"                                          );
	strcat (dialog_text_all, " Are you sure you want to delete this file? \n");
	strcat (dialog_text_all, "\n"                                            );
	strcat (dialog_text_all, "             Yes ○ ／ No ×\n"            );
	strcat (dialog_text_all, "\n"                                            );
	
	// ダイアログ表示
	dialog_y += 12;
	
	// 表示位置が下すぎたら上に表示
	if (dialog_y > 185) dialog_y -= 100;
	message_dialog (30, dialog_y, title_string, dialog_text_all);
	
	// 無限ループ気持ち悪い
	while (1) {
		readpad ();
		
		// The user has chosen to abort the delete
		if (new_pad & PSP_CTRL_CROSS) {
			return FALSE;
		}
		
		// The user as confirmed the file delete
		else if (new_pad & PSP_CTRL_CIRCLE){
			strcpy      (delete_path, path);
			strcat      (delete_path, name);
			delete_file (delete_path);
			return TRUE;
		}
	}
}

void delete_file (const char *path)
{
	sceIoRemove (path);
}

bool8 copy_file (const char *file)
{
	
}
