#include "shape.h"

extern SDL_Surface *Screen;
//-----------------------------------------------------------------------------
char Shape::getCell(int x, int y)
{
	if (x < 0 || x > 3 || y < 0 || y > 3)
		return 0;
	return dataM[x][y];
}
//-----------------------------------------------------------------------------
Uint32 Shape::getColor(int field)
{
	const Uint8 colors[8][3] = {
		  0,   0,   0,
		255,   0,   0,
		200, 100, 255,
		160, 255, 160,
		 60, 255,  60,
		255, 128,   0,
		  0, 255, 255,
		 60,  60, 255
	};

	if (field < 1 || field > 7)
		return 0;

	return SDL_MapRGB(Screen->format, colors[field][0], colors[field][1], colors[field][2]);
}
//-----------------------------------------------------------------------------
Shape::Shape(int shape, int rotation)
{
	static const char shapes[7][4][4] = {
		0,0,0,0,
		1,1,1,1,
		0,0,0,0,
		0,0,0,0,

		0,0,0,0,
		0,2,2,2,
		0,2,0,0,
		0,0,0,0,

		0,0,0,0,
		0,3,3,3,
		0,0,3,0,
		0,0,0,0,

		0,0,0,0,
		4,4,4,0,
		0,0,4,0,
		0,0,0,0,

		0,0,0,0,
		0,5,5,0,
		0,0,5,5,
		0,0,0,0,

		0,0,0,0,
		0,6,6,0,
		6,6,0,0,
		0,0,0,0,

		0,0,0,0,
		0,7,7,0,
		0,7,7,0,
		0,0,0,0
	};

	for (int x=0; x<4; ++x)
		for (int y=0; y<4; ++y)
			dataM[x][y] = shapes[shape][x][y];

	for (int i=0; i<rotation; ++i)
		Rotate();
}
//-----------------------------------------------------------------------------
void Shape::Rotate()
{
	char temp[4][4];
	for (int x=0; x<4; ++x)					// copy
		for (int y=0; y<4; ++y)
			temp[x][y] = dataM[x][y];

	for (int x=0; x<4; ++x)					// rotate
		for (int y=0; y<4; ++y)
			dataM[x][y] = temp[3-y][x];
}
//-----------------------------------------------------------------------------
