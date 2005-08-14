#ifndef AB_SHAPE_H
#define AB_SHAPE_H

#include "SDL.h"
//-----------------------------------------------------------------------------
class Shape
{
private:
	char dataM[4][4];
	void Rotate();
public:
	Shape(int shape, int rotation);
	char getCell(int x, int y);
	static Uint32 getColor(int field);
};
//-----------------------------------------------------------------------------
#endif
