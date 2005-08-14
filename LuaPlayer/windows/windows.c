#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include <pspctrl.h>

#include "../graphics.h"
#include "../framebuffer.h"

#define WIDTH 480
#define HEIGHT 272
static unsigned short fb[512 * HEIGHT * 2];
static unsigned char pixels[WIDTH * HEIGHT * 4];

int currentControls = 0;

typedef struct 
{
	int psp;
	char key;
} NormalKeyMapping;

NormalKeyMapping normalKeyMappings[] = {
	{ PSP_CTRL_SELECT, 'a' },
	{ PSP_CTRL_START, 's' },
	{ PSP_CTRL_UP, GLUT_KEY_UP },
	{ PSP_CTRL_RIGHT, GLUT_KEY_RIGHT },
	{ PSP_CTRL_DOWN, GLUT_KEY_DOWN },
	{ PSP_CTRL_LEFT, GLUT_KEY_LEFT },
	{ PSP_CTRL_LTRIGGER, 'q' },
	{ PSP_CTRL_RTRIGGER, 'w' },
	{ PSP_CTRL_TRIANGLE, 'r' },
	{ PSP_CTRL_CIRCLE, 'f' },
	{ PSP_CTRL_CROSS, 'c' },
	{ PSP_CTRL_SQUARE, 'd' },
	{ PSP_CTRL_HOME, ' ' },
	{ PSP_CTRL_HOLD, ' ' },
	{ PSP_CTRL_NOTE, ' ' },
	{ 0, 0 } };

typedef struct 
{
	int psp;
	int code;
} SpecialKeyMapping;

SpecialKeyMapping specialKeyMappings[] = {
	{ PSP_CTRL_UP, GLUT_KEY_UP },
	{ PSP_CTRL_RIGHT, GLUT_KEY_RIGHT },
	{ PSP_CTRL_DOWN, GLUT_KEY_DOWN },
	{ PSP_CTRL_LEFT, GLUT_KEY_LEFT },
	{ 0, 0 } };

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
}


void display(void)
{
	int i = 0;
	int x, y;
	u16* fb = getVramDisplayBuffer();
	for (y = HEIGHT - 1; y >= 0; y--) {
		for (x = 0; x < WIDTH; x++) {
			int color = fb[x + y * 512];
			unsigned char r = (color & 0x1f) << 3; 
			unsigned char g = ((color >> 5) & 0x1f) << 3 ;
			unsigned char b = ((color >> 10) & 0x1f) << 3 ;
			unsigned char a = color & 0x8000 ? 0xff : 0; 
			pixels[i++] = r;
			pixels[i++] = g;
			pixels[i++] = b;
			pixels[i++] = a;
		}
	}
	glDrawPixels(WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glutSwapBuffers();
}

void idle(void)
{
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 27) exit(0);
	int i = 0;
	while (1) {
		NormalKeyMapping mapping = normalKeyMappings[i++];
		if (mapping.psp == 0) break;
		if (mapping.key == key) currentControls |= mapping.psp;
	}
}

void keyboardUp(unsigned char key, int x, int y)
{
	int i = 0;
	while (1) {
		NormalKeyMapping mapping = normalKeyMappings[i++];
		if (mapping.psp == 0) break;
		if (mapping.key == key) currentControls &= ~mapping.psp;
	}
}

void special(int key, int x, int y)
{
	int i = 0;
	while (1) {
		SpecialKeyMapping mapping = specialKeyMappings[i++];
		if (mapping.psp == 0) break;
		if (mapping.code == key) currentControls |= mapping.psp;
	}
}

void specialUp(int key, int x, int y)
{
	int i = 0;
	while (1) {
		SpecialKeyMapping mapping = specialKeyMappings[i++];
		if (mapping.psp == 0) break;
		if (mapping.code == key) currentControls &= ~mapping.psp;
	}
}


DWORD WINAPI LuaThread(LPVOID pParam)
{
	runScript((char*) pParam);
	
	exit(0);
	return 0;
}

int main(int argc, char** argv)
{
	if (argc != 2) {
		printf("usage: luaplayer script.lua\n");
		return 1;
	}
	g_vram_base = fb;
	memset(fb, 0, WIDTH * HEIGHT * 2);

	initGraphics();
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(WIDTH, HEIGHT);
	
	glutCreateWindow("Lua Player");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(special);
	glutSpecialUpFunc(specialUp);
	
	glutIdleFunc(idle);
	DWORD threadID;
	HANDLE luaThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) LuaThread, argv[1], 0, &threadID);
	glutMainLoop();
	return 0;
}
