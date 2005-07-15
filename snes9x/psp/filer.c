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
//	if(bBitmap)
//		pgBitBlt(0,0,480,272,1,bgBitmap);
//	else
		pgFillvram(0x9063);
	mh_print(286, 0, (unsigned char *)" ■ uo_Snes9x for PSP Ver0.02pd1 ■", RGB(85,85,95));

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

	if(a->d_stat.st_mode==b->d_stat.st_mode){
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
	
	if(a->d_stat.st_mode & FIO_SO_IFDIR)	return -1;
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

// 拡張子管理用
const struct {
	char *szExt;
	int   nExtId;
} stExtentions [] = {
	{ "sfc", EXT_SFC     },
	{ "smc", EXT_SMC     },
	{ "zip", EXT_ZIP     },
	{   0,   EXT_UNKNOWN }
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
	
	nfiles = 0;
	memset (files, 0, sizeof(files));
	
	if(strcmp(path,"ms0:/")){
		strcpy(files[nfiles].d_name,"..");
		files[nfiles].d_stat.st_mode = FIO_SO_IFDIR;
		nfiles++;
		b=1;
	}
	
	fd = sceIoDopen(path);
	while(nfiles<MAX_ENTRY){
		if(sceIoDread(fd, &files[nfiles])<=0) break;
		if(files[nfiles].d_name[0] == '.') continue;
		if(files[nfiles].d_stat.st_mode == FIO_SO_IFDIR){
			strcat(files[nfiles].d_name, "/");
			nfiles++;
			continue;
		}
		if(getExtId(files[nfiles].d_name) != EXT_UNKNOWN) nfiles++;
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
	int top, rows=21, x, y, h, i, bMsg=0, up=0;
	char path[_MAX_PATH], oldDir[_MAX_PATH], *p;
// add by J
	int  dialog_y = 0;
	
	top = sel-3;
	
	strcpy(path, LastPath);
	if(FilerMsg[0])
		bMsg=1;
	
	getDir(path);

	for(;;){
		readpad ();

		if(new_pad)
			bMsg=0;
		if(new_pad & PSP_CTRL_CIRCLE){
			if(files[sel].d_stat.st_mode == FIO_SO_IFDIR){
				if(!strcmp(files[sel].d_name,"..")){
					up=1;
				}else{
					strcat(path,files[sel].d_name);
					getDir(path);
					sel=0;
				}
			}else{
				strcpy(out, path);
				strcat(out, files[sel].d_name);
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
// add by a
		}else if(new_pad & PSP_CTRL_LTRIGGER){
			sel-=20;
 		}else if(new_pad & PSP_CTRL_RTRIGGER){
			sel+=20;
// add by J
		// ROM削除(ロボタンでROM削除)
		}else if( new_pad & PSP_CTRL_SQUARE ) {
			if( files[sel].d_stat.st_mode != FIO_SO_IFDIR) {
				if( strcmp( files[sel].d_name , ".." ) ) {
					//   ループカウント
					int  loop_cnt;
					//   ROM削除用パス
					char delete_path[ _MAX_PATH ];
					//   タイトル文字列
					char title_string[ D_text_MAX ];
					//   ダイアログテキストの保存用変数
					char dialog_text_all[ D_text_all_MAX ];
					// ダイアログタイトル文字列作成
					strcpy( title_string,    " 　【　　ROM Delete　　】　 \0" );
					// メッセージ作成
					strcpy( dialog_text_all, files[sel].d_name                );
					strcat( dialog_text_all, "\n\n\0"                         ); // 改行x2
					strcat( dialog_text_all, "     ROMを削除します。\n\0"     );
					strcat( dialog_text_all, "     よろしいですか？\n\0"      );
					strcat( dialog_text_all, "\n\0"                           ); // 改行
					strcat( dialog_text_all, "   はい ○ ／ いいえ ×\n\0"    );
					strcat( dialog_text_all, "\n\0"                           ); // 改行
					// ダイアログ表示
					dialog_y += 12;
					// 表示位置が下すぎたら上に表示
					if ( dialog_y > 185 ) dialog_y -= 100;
					message_dialog( 30, dialog_y, title_string, dialog_text_all );
					// 無限ループ気持ち悪い
					while( 1 ){
						readpad();
						// ×ボタン押した
						if ( new_pad & PSP_CTRL_CROSS ) {
							break;
						// ○ボタン押す(ROM削除)
						} else if ( new_pad & PSP_CTRL_CIRCLE ){
							strcpy( delete_path, path);
							strcat( delete_path, files[ sel ].d_name );
							sceIoRemove( delete_path );
							getDir(path);
							break;
						}
					}
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
				getDir(path);
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
		
		if(bMsg)
			menu_frame((unsigned char *)FilerMsg,(unsigned char *)"○：OK　×：CANCEL　△：UP　□：DELETE");
		else
			menu_frame((unsigned char *)path,(unsigned char *)"○：OK　×：CANCEL　△：UP　□：DELETE");
		
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
// mod by J
// 赤に変えました
//			if(top+i == sel) color = RGB(105,105,115);
			if(top+i == sel) {
				color = RGB( 255,  0,   0 );
				dialog_y = y;
			}else {
				color = 0xffff;
			}
			mh_print(x, y, (unsigned char *)files[top+i].d_name, color);
			y+=10;
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
	// プログレス
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

void delete_file(const char *path)
{
	sceIoRemove(path);
}

