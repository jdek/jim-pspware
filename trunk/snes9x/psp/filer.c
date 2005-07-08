#include "psp.h"
#include "pg.h"
#include "filer.h"
#include "psp/zlibInterface.h"

#define true 1
#define false 0

extern u32 new_pad;

struct dirent files[MAX_ENTRY];
int nfiles;

void menu_frame(const char *msg0, const char *msg1)
{
//	if(bBitmap)
//		pgBitBlt(0,0,480,272,1,bgBitmap);
//	else
		pgFillvram(0x9063);
	mh_print(286, 0, " ■ uo_Snes9x for PSP Ver0.02y11 ■", RGB(85,85,95));
	// メッセージなど
	if(msg0!=0) mh_print(17, 14, msg0, RGB(105,105,115));
	pgDrawFrame(17,25,463,248,RGB(85,85,95));
	pgDrawFrame(18,26,462,247,RGB(85,85,95));
	// 操作説明
	if(msg1!=0) mh_print(17, 252, msg1, RGB(105,105,115));
}

void SJISCopy(struct dirent *a, unsigned char *file)
{
	unsigned char ca;
	int i;

	for(i=0;i<=strlen(a->name);i++){
		ca = a->name[i];
		if (((0x81 <= ca)&&(ca <= 0x9f))
		|| ((0xe0 <= ca)&&(ca <= 0xef))){
			file[i++] = ca;
			file[i] = a->name[i];
		}
		else{
			if(ca>='a' && ca<='z') ca-=0x20;
			file[i] = ca;
		}
	}

}
int cmpFile(struct dirent *a, struct dirent *b)
{
    unsigned char file1[0x108];
    unsigned char file2[0x108];
	unsigned char ca, cb;
	int i, n, ret;

	if(a->type==b->type){
		SJISCopy(a, file1);
		SJISCopy(b, file2);
		n=strlen(file1);
		for(i=0; i<=n; i++){
			ca=file1[i]; cb=file2[i];
			ret = ca-cb;
			if(ret!=0) return ret;
		}
		return 0;
	}
	
	if(a->type & TYPE_DIR)	return -1;
	else					return 1;
}

void sort(struct dirent *a, int left, int right) {
	struct dirent tmp, pivot;
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

// 拡張子管理用
const struct {
	char *szExt;
	int nExtId;
} stExtentions[] = {
 "sfc",EXT_SFC,
 "smc",EXT_SMC,
 "zip",EXT_ZIP,
 0, EXT_UNKNOWN
};

int getExtId(const char *szFilePath) {
	char *pszExt;
	int i;
	if((pszExt = (char *)strrchr(szFilePath, '.'))) {
		pszExt++;
		for (i = 0; stExtentions[i].nExtId != EXT_UNKNOWN; i++) {
			if (!stricmp(stExtentions[i].szExt,pszExt)) {
				return stExtentions[i].nExtId;
			}
		}
	}
	return EXT_UNKNOWN;
}

void getDir(const char *path) {
	int fd, b=0;
	char *p;
	
	nfiles = 0;
	memset (files, 0, sizeof(files));
	
	if(strcmp(path,"ms0:/")){
		strcpy(files[nfiles].name,"..");
		files[nfiles].type = TYPE_DIR;
		nfiles++;
		b=1;
	}
	
	fd = sceIoDopen(path);
	while(nfiles<MAX_ENTRY){
		if(sceIoDread(fd, &files[nfiles])<=0) break;
		if(files[nfiles].name[0] == '.') continue;
		if(files[nfiles].type == TYPE_DIR){
			strcat(files[nfiles].name, "/");
			nfiles++;
			continue;
		}
		if(getExtId(files[nfiles].name) != EXT_UNKNOWN) nfiles++;
	}
	sceIoDclose(fd);
	if(b)
		sort(files+1, 0, nfiles-2);
	else
		sort(files, 0, nfiles-1);
}

char LastPath[_MAX_PATH];
char FilerMsg[256];
int getFilePath(char *out)
{
	unsigned long color;
	static int sel=0;
	int top, rows=21, x, y, h, i, len, bMsg=0, up=0;
	char path[_MAX_PATH], oldDir[_MAX_PATH], *p;
	
	top = sel-3;
	
	strcpy(path, LastPath);
	if(FilerMsg[0])
		bMsg=1;
	
	getDir(path);
	readpad();
	for(;;){
		if(new_pad)
			bMsg=0;
		if(new_pad & PSP_CTRL_CIRCLE){
			if(files[sel].type == TYPE_DIR){
				if(!strcmp(files[sel].name,"..")){
					up=1;
				}else{
					strcat(path,files[sel].name);
					getDir(path);
					sel=0;
				}
			}else{
				strcpy(out, path);
				strcat(out, files[sel].name);
				strcpy(LastPath,path);
				return 1;
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
				getDir(path);
				sel=0;
				for(i=0; i<nfiles; i++) {
					if(!strcmp(oldDir, files[i].name)) {
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
		
		if(bMsg)
			menu_frame(FilerMsg,"○：OK　×：CANCEL　△：UP");
		else
			menu_frame(path,"○：OK　×：CANCEL　△：UP");
		
		// スクロールバー
		if(nfiles > rows){
			h = 219;
			pgDrawFrame(445,25,446,248,RGB(85,85,95));
			pgFillBox(448, h*top/nfiles + 27,
				460, h*(top+rows)/nfiles + 27,RGB(85,85,95));
		}
		
		x=28; y=32;
		for(i=0; i<rows; i++){
			if(top+i >= nfiles) break;
			if(top+i == sel) color = RGB(105,105,115);
			else			 color = 0xffff;
			mh_print(x, y, files[top+i].name, color);
			y+=10;
		}
		do readpad(); while(!new_pad);
	}
}

// せっかくなのでプログレスでも出してみます
void draw_load_rom_progress(unsigned long ulExtractSize, unsigned long ulCurrentPosition)
{
	int nPer = 100 * ulExtractSize / ulCurrentPosition;
	static int nOldPer = 0;
	if (nOldPer == nPer & 0xFFFFFFFE) {
		return ;
	}
	nOldPer = nPer;
	pgFillvram(0x9063);
	// プログレス
	pgDrawFrame(89,121,391,141,RGB(85,85,95));
	pgFillBox(90,123, 90+nPer*3, 139,RGB(85,85,95));
	// ％
	char szPer[16];
	itoa(nPer, szPer);
	strcat(szPer, "%");
	pgPrint(28,16,0xffff,szPer);
	// pgScreenFlipV()を使うとpgWaitVが呼ばれてしまうのでこちらで。
	// プログレスだからちらついても良いよね～
	pgScreenFlip();
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

unsigned char slotflag[SAVE_SLOT_MAX+1];
char slotdate[SAVE_SLOT_MAX+1][32];
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
		strcpy(slotdate[i],".sv0 - ");
		slotdate[i][3] = name[strlen(name)-1] = i + '0';
		for(j=0; j<nfiles; j++){
			if(!stricmp(name,files[j].name)){
				ustoa(files[j].mtime.year,tmp);
				strcat(slotdate[i],tmp);
				strcat(slotdate[i],"/");
				
				if(files[j].mtime.mon < 10) strcat(slotdate[i],"0");
				ustoa(files[j].mtime.mon,tmp);
				strcat(slotdate[i],tmp);
				strcat(slotdate[i],"/");
				
				if(files[j].mtime.mday < 10) strcat(slotdate[i],"0");
				ustoa(files[j].mtime.mday,tmp);
				strcat(slotdate[i],tmp);
				strcat(slotdate[i]," ");
				
				if(files[j].mtime.hour < 10) strcat(slotdate[i],"0");
				ustoa(files[j].mtime.hour,tmp);
				strcat(slotdate[i],tmp);
				strcat(slotdate[i],":");
				
				if(files[j].mtime.min < 10) strcat(slotdate[i],"0");
				ustoa(files[j].mtime.min,tmp);
				strcat(slotdate[i],tmp);
				strcat(slotdate[i],":");
				
				if(files[j].mtime.sec < 10) strcat(slotdate[i],"0");
				ustoa(files[j].mtime.sec,tmp);
				strcat(slotdate[i],tmp);
				
				slotflag[i] = true;
				break;
			}
		}
		if(j>=nfiles){
			strcat(slotdate[i],"not exist");
			slotflag[i] = false;
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
	
	vptr0=buf;
	for (y=0; y<224; y+=2) {
		rgbptr=(unsigned short *)vptr0;
		for (x=0; x<256; x+=2) {
			rgb = (*rgbptr & *(rgbptr+1)) + (((*rgbptr ^ *(rgbptr+1)) & 0x7bde) >> 1);
			rgb2 = (*(rgbptr+LINESIZE) & *(rgbptr+LINESIZE+1)) + (((*(rgbptr+LINESIZE) ^ *(rgbptr+LINESIZE+1)) & 0x7bde) >> 1);
			now_thumb[i++] = (rgb & rgb2) + (((rgb ^ rgb2) & 0x7bde) >> 1);
			rgbptr+=2;
		}
		vptr0+=LINESIZE*4;
	}
}

unsigned char thumbflag[SAVE_SLOT_MAX+1];
unsigned short slot_thumb[SAVE_SLOT_MAX+1][128 * 112];
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
			if(!stricmp(name,files[j].name)){
				strcpy(filepath, path);
				strcat(filepath, name);
				fd = sceIoOpen( filepath, PSP_O_RDONLY, 0777 );
				if (fd>=0) {
					sceIoRead(fd, &slot_thumb[i], sizeof(slot_thumb[0]));
					sceIoClose(fd);
				}
				thumbflag[i] = true;
				break;
			}
		}
		if(j>=nfiles){
			memset (slot_thumb[i], 0, sizeof(slot_thumb[0]));
			thumbflag[i] = false;
		}
	}
}

void save_thumb(const char *path)
{
	int fd, size;
	fd = sceIoOpen( path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777 );
	if (fd>=0) {
		size = sceIoWrite(fd, &now_thumb, sizeof(now_thumb));
		sceIoClose(fd);
	}
	if (size == sizeof(now_thumb)) return;
	sceIoRemove(path);
}

void delete_file(const char *path)
{
	sceIoRemove(path);
}

