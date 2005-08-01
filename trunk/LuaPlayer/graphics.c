#include <stdlib.h>
#include <malloc.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <png.h>
#include <pspgu.h>

#include "graphics.h"

#define	PSP_LINE_SIZE 512
#define IS_ALPHA(color) ((color)&0x8000?0:1)
#define FRAMEBUFFER_SIZE (PSP_LINE_SIZE*SCREEN_HEIGHT*2)
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define VRAM_BASE (0x40000000 | 0x04000000)

typedef struct 
{
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
} Vertex;

extern u8 msx[];

static unsigned int __attribute__((aligned(16))) list[256];
static int dispBufferNumber;
static int initialized = 0;

static void setWidthToNextPower2(Image* image)
{
	int width = MAX(image->imageWidth, image->imageHeight);
	int b = width;
	int n;
	for (n = 0; b != 0; n++) b >>= 1;
	image->textureWidth = 1 << n;
	if (image->textureWidth == 2 * width) image->textureWidth >>= 1;
}

static u16* getVramDrawBuffer()
{
	u16* vram = (u16*) VRAM_BASE;
	if (dispBufferNumber == 0) vram += FRAMEBUFFER_SIZE / 2;
	return vram;
}

static u16* getVramDisplayBuffer()
{
	u16* vram = (u16*) VRAM_BASE;
	if (dispBufferNumber == 1) vram += FRAMEBUFFER_SIZE / 2;
	return vram;
}

Image* loadImage(const char* filename)
{
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, x, y;
	u32* line;
	FILE *fp;
	Image* image = (Image*) malloc(sizeof(Image));
	if (!image) return NULL;

	if ((fp = fopen(filename, "rb")) == NULL) return NULL;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(fp);
		return NULL;;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	image->imageWidth = width;
	image->imageHeight = height;
	setWidthToNextPower2(image);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	image->data = (u16*) memalign(16, image->textureWidth * image->textureWidth * 2);
	if (!image->data) return NULL;
	line = (u32*) malloc(width * 4);
	for (y = 0; y < height; y++) {
		png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
		for (x = 0; x < width; x++) {
			u32 color = line[x];
			int r = color & 0xff;
			int g = (color >> 8) & 0xff;
			int b = (color >> 16) & 0xff;
			u16 color1555 = (r >> 3) | ((g >> 3)<<5) | ((b >> 3)<<10);
			if ((color & 0xff000000) != 0 ) color1555 |= 0x8000;
			image->data[x + y * image->textureWidth] = color1555;
		}
	}
	free(line);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);
	sceKernelDcacheWritebackInvalidateAll();
	return image;
}

void blitImageToImage(int sx, int sy, int width, int height, Image* source, int dx, int dy, Image* destination)
{
	int x, y;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			destination->data[destination->textureWidth * (y + dy) + x + dx] =
				source->data[source->textureWidth * (y + sy) + x + sx];
		}
	}
	sceKernelDcacheWritebackInvalidateAll();
}

void blitImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy)
{
	if (!initialized) return;
	u16* vram = getVramDrawBuffer();
	sceGuStart(GU_DIRECT,list);
	sceGuCopyImage(GU_PSM_5551, sx, sy, width, height, source->textureWidth, source->data, dx, dy, PSP_LINE_SIZE, vram);
	sceGuFinish();
	sceGuSync(0,0);
}

void blitAlphaImageToImage(int sx, int sy, int width, int height, Image* source, int dx, int dy, Image* destination)
{
	int x, y;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			u16 color = source->data[source->textureWidth * (y + sy) + x + sx];
			if (!IS_ALPHA(color)) {
				destination->data[destination->textureWidth * (y + dy) + x + dx] = color;
			}
		}
	}
	sceKernelDcacheWritebackInvalidateAll();
}

void blitAlphaImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy)
{
	if (!initialized) return;

	sceGuStart(GU_DIRECT, list);
	sceGuTexImage(0, source->textureWidth, source->textureWidth, source->textureWidth, (void*) source->data);
	float w = 1.0f / ((float)source->textureWidth);
	sceGuTexScale(w, w);
	
	int j = 0;
	while (j < width) {
		Vertex* vertices = (Vertex*) sceGuGetMemory(2 * sizeof(Vertex));
		int sliceWidth = 64;
		if (j + sliceWidth > width) sliceWidth = width - j;
		vertices[0].u = sx + j;
		vertices[0].v = sy;
		vertices[0].color = 0;
		vertices[0].x = dx + j;
		vertices[0].y = dy;
		vertices[0].z = 0;
		vertices[1].u = sx + j + sliceWidth;
		vertices[1].v = sy + height;
		vertices[1].color = 0;
		vertices[1].x = dx + j + sliceWidth;
		vertices[1].y = dy + height;
		vertices[1].z = 0;
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_COLOR_5551 | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
		j += sliceWidth;
	}
	
	sceGuFinish();
	sceGuSync(0, 0);
}

Image* createImage(int width, int height)
{
	Image* image = (Image*) malloc(sizeof(Image));
	if (!image) return NULL;
	image->imageWidth = width;
	image->imageHeight = height;
	setWidthToNextPower2(image);
	image->data = (u16*) memalign(16, image->textureWidth * image->textureWidth * 2);
	if (!image->data) return NULL;
	memset(image->data, 0, image->textureWidth * image->textureWidth * 2);
	sceKernelDcacheWritebackInvalidateAll();
	return image;
}

void clearImage(u16 color, Image* image)
{
	memset(image->data, color, image->textureWidth * image->textureWidth * 2);
	sceKernelDcacheWritebackInvalidateAll();
}

void clearScreen(u16 color)
{
	if (!initialized) return;
	memset(getVramDrawBuffer(), color, PSP_LINE_SIZE * SCREEN_HEIGHT * 2);
	sceKernelDcacheWritebackInvalidateAll();
}

void fillImageRect(u16 color, int x0, int y0, int width, int height, Image* image)
{
	int x, y;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			image->data[x + x0 + (y + y0) * image->textureWidth] = color;
		}
	}
	sceKernelDcacheWritebackInvalidateAll();
}

void fillScreenRect(u16 color, int x0, int y0, int width, int height)
{
	sceKernelDcacheWritebackInvalidateAll();
	if (!initialized) return;
	u16* vram = getVramDrawBuffer();
	int x, y;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			vram[PSP_LINE_SIZE * (y + y0) + x + x0] = color;
		}
	}
	sceKernelDcacheWritebackInvalidateAll();
}

void putPixelScreen(u16 color, int x, int y)
{
	u16* vram = getVramDrawBuffer();
	vram[PSP_LINE_SIZE * y + x] = color;
	sceKernelDcacheWritebackInvalidateAll();
}

void putPixelImage(u16 color, int x, int y, Image* image)
{
	image->data[x + y * image->textureWidth] = color;
	sceKernelDcacheWritebackInvalidateAll();
}

u16 getPixelScreen(int x, int y)
{
	u16* vram = getVramDrawBuffer();
	return vram[PSP_LINE_SIZE * y + x];
}

u16 getPixelImage(int x, int y, Image* image)
{
	return image->data[x + y * image->textureWidth];
}

static const char digitFont[] = {
	1,1,1,1,
	1,0,0,1,
	1,0,0,1,
	1,0,0,1,
	1,1,1,1,
	
	0,0,0,1,
	0,0,0,1,
	0,0,0,1,
	0,0,0,1,
	0,0,0,1,
	
	1,1,1,1,
	0,0,0,1,
	1,1,1,1,
	1,0,0,0,
	1,1,1,1,
	
	1,1,1,1,
	0,0,0,1,
	1,1,1,1,
	0,0,0,1,
	1,1,1,1,
	
	1,0,0,1,
	1,0,0,1,
	1,1,1,1,
	0,0,0,1,
	0,0,0,1,
	
	1,1,1,1,
	1,0,0,0,
	1,1,1,1,
	0,0,0,1,
	1,1,1,1,
	
	1,1,1,1,
	1,0,0,0,
	1,1,1,1,
	1,0,0,1,
	1,1,1,1,
	
	1,1,1,1,
	0,0,0,1,
	0,0,0,1,
	0,0,0,1,
	0,0,0,1,
	
	1,1,1,1,
	1,0,0,1,
	1,1,1,1,
	1,0,0,1,
	1,1,1,1,

	1,1,1,1,
	1,0,0,1,
	1,1,1,1,
	0,0,0,1,
	1,1,1,1};

void print7SegmentScreen(int x, int y, int digit, u32 color)
{
	if (!initialized) return;
	u16* vram = getVramDrawBuffer();
	int xo, yo;
	int i = digit * 5*4;
	for (yo = 0; yo < 5; yo++) {
		for (xo = 0; xo < 4; xo++) {
			if (digitFont[i]) {
				vram[xo + x + (yo + y) * PSP_LINE_SIZE] = color;
			}
			i++;
		}
	}
	sceKernelDcacheWritebackInvalidateAll();
}

void print7SegmentImage(int x, int y, int digit, u32 color, Image* image)
{
	if (!initialized) return;
	int xo, yo;
	int i = digit * 5*4;
	for (yo = 0; yo < 5; yo++) {
		for (xo = 0; xo < 4; xo++) {
			if (digitFont[i]) {
				image->data[xo + x + (yo + y) * image->textureWidth] = color;
			}
			i++;
		}
	}
	sceKernelDcacheWritebackInvalidateAll();
}

void printTextScreen(int x, int y, const char* text, u32 color)
{
	int c, i, j, l;
	u8 *font;
	u16 *vram_ptr;
	u16 *vram;
	
	if (!initialized) return;

	for (c = 0; c < strlen(text); c++) {
		if (x < 0 || x + 8 >= SCREEN_WIDTH || y < 0 || y + 8 >= SCREEN_HEIGHT) break;
		char ch = text[c];
		vram = getVramDrawBuffer() + x + y * PSP_LINE_SIZE;
		
		font = &msx[ (int)ch * 8];
		for (i = l = 0; i < 8; i++, l += 8, font++) {
			vram_ptr  = vram;
			for (j = 0; j < 8; j++) {
				if ((*font & (128 >> j))) *vram_ptr = color;
				vram_ptr++;
			}
			vram += PSP_LINE_SIZE;
		}
		x += 8;
	}
	sceKernelDcacheWritebackInvalidateAll();
}

void printTextImage(int x, int y, const char* text, u32 color, Image* image)
{
	int c, i, j, l;
	u8 *font;
	u16 *data_ptr;
	u16 *data;
	
	if (!initialized) return;

	for (c = 0; c < strlen(text); c++) {
		if (x < 0 || x + 8 >= image->imageWidth || y < 0 || y + 8 >= image->imageHeight) break;
		char ch = text[c];
		data = image->data + x + y * image->textureWidth;
		
		font = &msx[ (int)ch * 8];
		for (i = l = 0; i < 8; i++, l += 8, font++) {
			data_ptr  = data;
			for (j = 0; j < 8; j++) {
				if ((*font & (128 >> j))) *data_ptr = color;
				data_ptr++;
			}
			data += image->textureWidth;
		}
		x += 8;
	}
	sceKernelDcacheWritebackInvalidateAll();
}

void screenshot(const char* filename)
{
	const char tgaHeader[] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	u16 lineBuffer[SCREEN_WIDTH];
	u16* vram = getVramDisplayBuffer();
	int x, y;
	FILE* file = fopen(filename, "wb");
	if (!file) return;
	fwrite(tgaHeader, sizeof(tgaHeader), 1, file);
	fputc(SCREEN_WIDTH & 0xff, file);
	fputc(SCREEN_WIDTH >> 8, file);
	fputc(SCREEN_HEIGHT & 0xff, file);
	fputc(SCREEN_HEIGHT >> 8, file);
	fputc(16, file);
	fputc(0, file);
	for (y = SCREEN_HEIGHT - 1; y >= 0; y--) {
		for (x = 0; x < SCREEN_WIDTH; x++) {
			u16 color = vram[y * PSP_LINE_SIZE + x];
			int red = color & 0x1f;
			int green = (color >> 5) & 0x1f;
			int blue = (color >> 10) & 0x1f;
			lineBuffer[x] = blue | (green<<5) | (red<<10);
		}
		fwrite(lineBuffer, SCREEN_WIDTH, 2, file);
	}
	fclose(file);
}

void flipScreen()
{
	if (!initialized) return;
	u16* vram = getVramDrawBuffer();
	sceGuSwapBuffers();
	sceDisplaySetFrameBuf(vram, PSP_LINE_SIZE, 1, 1);
	dispBufferNumber ^= 1;
}

static void drawLine(int x0, int y0, int x1, int y1, int color, u16* destination, int width)
{
	#define SWAP(a, b) tmp = a; a = b; b = tmp;
	int x, y, e, dx, dy, tmp;
	if (x0 > x1) {
		SWAP(x0, x1);
		SWAP(y0, y1);
	}
	e = 0;
	x = x0;
	y = y0;
	dx = x1 - x0;
	dy = y1 - y0;
	if (dy >= 0) {
		if (dx >= dy) {
			for (x = x0; x <= x1; x++) {
				destination[x + y * width] = color;
				if (2 * (e + dy) < dx) {
					e += dy;
				} else {
					y++;
					e += dy - dx;
				}
			}
		} else {
			for (y = y0; y <= y1; y++) {
				destination[x + y * width] = color;
				if (2 * (e + dx) < dy) {
					e += dx;
				} else {
					x++;
					e += dx - dy;
				}
			}
		}
	} else {
		if (dx >= -dy) {
			for (x = x0; x <= x1; x++) {
				destination[x + y * width] = color;
				if (2 * (e + dy) > -dx) {
					e += dy;
				} else {
					y--;
					e += dy + dx;
				}
			}
		} else {   	
			SWAP(x0, x1);
			SWAP(y0, y1);
			x = x0;
			dx = x1 - x0;
			dy = y1 - y0;
			for (y = y0; y <= y1; y++) {
				destination[x + y * width] = color;
				if (2 * (e + dx) > -dy) {
					e += dx;
				} else {
					x--;
					e += dx + dy;
				}
			}
		}
	}
}

void drawLineScreen(int x0, int y0, int x1, int y1, int color)
{
	drawLine(x0, y0, x1, y1, color, getVramDrawBuffer(), PSP_LINE_SIZE);
}

void drawLineImage(int x0, int y0, int x1, int y1, int color, Image* image)
{
	drawLine(x0, y0, x1, y1, color, image->data, image->textureWidth);
}

void initGraphics()
{
	sceDisplaySetMode(0,SCREEN_WIDTH,SCREEN_HEIGHT);

	dispBufferNumber = 0;
	sceDisplayWaitVblankStart();
	sceDisplaySetFrameBuf((void*) VRAM_BASE, PSP_LINE_SIZE, 1, 1);

	sceGuInit();

	sceGuStart(GU_DIRECT, list);
	sceGuDrawBuffer(GU_PSM_5551, (void*)FRAMEBUFFER_SIZE, PSP_LINE_SIZE);
	sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, (void*)0, PSP_LINE_SIZE);
	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
	sceGuDepthBuffer((void*) 0x110000, PSP_LINE_SIZE);
	sceGuOffset(2048 - (SCREEN_WIDTH / 2), 2048 - (SCREEN_HEIGHT / 2));
	sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceGuDepthRange(0xc350, 0x2710);
	sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuAlphaFunc(GU_GREATER, 0, 0xff);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuTexMode(GU_PSM_5551, 0, 0, 0);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	sceGuAmbientColor(0xffffffff);
	sceGuFinish();
	sceGuSync(0, 0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);
	initialized = 1;
}

void disableGraphics()
{
	initialized = 0;
}
