#include <SDL.h>
#include "sutils.h"
#include "game.h"
#include "shape.h"
#include "config.h"
#include "player.h"
//-----------------------------------------------------------------------------
extern SDL_Surface *Screen;		// global object
//-----------------------------------------------------------------------------
void Player::SetupLevel()
{
	canPlayM = true;
	for (int x = 0; x<fieldWidth; ++x)
		for (int y = 0; y<fieldHeight; ++y)
			fieldM[x][y] = 0;
	nextShapeM = gameM->getRandomShape();
	NextBrick();
}
//-----------------------------------------------------------------------------
void Player::SetupGame(Game *g, int x, int y, int xs, int ys)
{
	countdown = 25;
	speed_coef = 250;
	gameM = g;
	pointsM = 0;
	rowsM = 0;
	xoffset = x;
	yoffset = y;
	xstats = xs;
	ystats = ys;
}
//-----------------------------------------------------------------------------
//! returns false if new brick cannot be placed
bool Player::Gravity(bool force)
{
	countdown--;
	if (force)
		countdown = 0;
	if (countdown < 1)
	{
		countdown = (int)((double)speed_coef * 0.1);
		if (!CheckPieceMove(xcoordM, ycoordM+1))	// cannot move
		{
			gameM->playSound(Game::tDrop);
			for (int x=0; x<4; ++x)					// place the brick on the heap
			{
				for (int y=0; y<4; ++y)
				{
					char c = getCell(x,y,currentShapeM);
					if (c)
						fieldM[xcoordM+x][ycoordM+y] = c;
				}
			}
			CheckLine();
			canPlayM = NextBrick();
			if (!canPlayM)
				return false;
		}
		else
			ycoordM++;
	}
	return true;
}
//-----------------------------------------------------------------------------
//! returns false if brick cannot be placed
bool Player::NextBrick()
{
	int gameType = 0;
	config().getValue("game_type", gameType);

	if (gameType == 2)	// bastet
		currentShapeM = gameM->getRandomShape();
	else
	{
		currentShapeM = nextShapeM;
		nextShapeM = gameM->getRandomShape();
	}

	rotationM = 0;
	xcoordM = 3;
	ycoordM = 0;
	while (true)
	{
		bool exit = false;
		for (int x=0; x<4; x++)
			if (getCell(x, -ycoordM, currentShapeM))
				exit = true;
		if (exit)
			break;
		ycoordM--;
	}

	if (gameType == 0)	// challenge game type
		pointsM++;
	else if (gameType == 1)
		pointsM += 3;
	else
		pointsM += 20;

	return CheckPieceMove(xcoordM, ycoordM);
}
//-----------------------------------------------------------------------------
//! returns number of lines dropped
int Player::CheckLine(char (*field)[fieldWidth][fieldHeight])
{
	if (field == 0)
		field = &fieldM;

	int lines_dropped = 0;
	int drop_pos = 0;

	// check to see if some line is full
	for (int y = fieldHeight-1; y > 0; --y)
	{
		bool line_full = true;
		for (int x = 0; x < fieldWidth; ++x)
		{
			if ((*field)[x][y] == 0)
			{
				line_full = false;
				break;
			}
		}
		if (!line_full)
			continue;

		lines_dropped++;
		drop_pos = y;
		for (int cy = y; cy > 0; --cy)
			for (int cx = 0; cx < fieldWidth; ++cx)
				(*field)[cx][cy] = (*field)[cx][cy-1];
		for (int cx = 0; cx < fieldWidth; ++cx)
			(*field)[cx][0] = 0;
		y++;	// stay on the same line
	}

	if (field == &fieldM)
	{
		rowsM += lines_dropped;

		if (lines_dropped && gameM)
		{
			speed_coef -= 2;
			if (speed_coef < 35)
				speed_coef = 35;
			int gameType = 0, coef = 5;
			config().getValue("game_type", gameType);
			if (gameType == 1)	// challenge game type
				coef = 3;
			if (gameType == 2)	// bastet
				coef = 100;
			int add_points = lines_dropped * lines_dropped * coef;
			pointsM += add_points;
			if (gameM->linesDropped(this, lines_dropped))
			{
				ScoreEffect *s = new ScoreEffect(gameM->getFont(), xoffset+fieldWidth*16+5, yoffset+(drop_pos)*16, add_points);
				gameM->specialEffects.addEffect(s);
			}
		}
	}
	return lines_dropped;
}
//-----------------------------------------------------------------------------
//! returns whether the piece can be placed at x,y coords
bool Player::CheckPieceMove(int atx, int aty)
{
	for (int x=0; x<4; x++)
	{
		for (int y=0; y<4; y++)
		{
			if (!getCell(x, y, currentShapeM))
				continue;

			int xc = atx+x;
			int yc = aty+y;
			if (xc >= fieldWidth || xc < 0 || yc >= fieldHeight || yc < 0)
				return false;

			if (fieldM[xc][yc])
				return false;
		}
	}
	return true;
}
//-----------------------------------------------------------------------------
//! returns cell for position x,y using rotationM and currentShapeM
char Player::getCell(int x, int y, int shape, int rotation)
{
	if (rotation == -1)
		rotation = rotationM;
	Shape s(shape, rotation);
	char c = s.getCell(x, y);
	return c;
}
//-----------------------------------------------------------------------------
//! returns whether the piece can be placed at x,y coords
//! if it can be placed to the left/right it allows it
bool Player::CheckPieceRotate(int& atx, int& aty)
{
	if (CheckPieceMove(atx,aty))	// can be placed regularly
		return true;

	// if next to obstacle try moving left/right
	bool is_left = false;
	for (int x=0; x<2 && !is_left; x++)
	{
		for (int y=0; y<4; y++)
		{
			if (!getCell(x, y, currentShapeM))
				continue;

			int xc = atx+x;
			int yc = aty+y;
			if (yc >= fieldHeight)
				break;
			if (xc < 0 || fieldM[xc][yc])
			{
				is_left = true;
				break;
			}
		}
	}
	if (is_left)
	{
		for (int x = 0; x<3; ++x)
		{
			if (CheckPieceMove(atx+x, aty))
			{
				atx += x;
				return true;
			}
		}
	}

	bool is_right = false;
	for (int x=2; x<4 && is_right == 0; x++)
	{
		for (int y=0; y<4; y++)
		{
			if (!getCell(x, y, currentShapeM))
				continue;

			int xc = atx+x;
			int yc = aty+y;
			if (yc >= fieldHeight)
				break;
			if (xc >= fieldWidth || fieldM[xc][yc])
			{
				is_right = true;
				break;
			}
		}
	}
	if (is_right)
	{
		for (int x = 0; x<3; ++x)
		{
			if (CheckPieceMove(atx-x, aty))
			{
				atx -= x;
				return true;
			}
		}
	}

	// if over the top try moving down
	if (aty < 0)
	{
		for (int y = 1; y <= -aty; ++y)
		{
			if (CheckPieceMove(atx, aty + y))
			{
				aty += y;
				countdown = (int)((double)speed_coef * 0.1);	// reset the countdown since it was forcedly pushed down
				return true;
			}
		}
	}

	return false;
}
//-----------------------------------------------------------------------------
void Player::LevelComplete()		// render nice animation & award bonus points
{
	Uint32 color = SDL_MapRGB(Screen->format, 100, 80, 0);
	for (int yo=0; ; ++yo)
	{
		gameM->playSound(Game::tDrop);

		for (int x=0; x < fieldWidth; ++x)
		{
			if (fieldM[x][yo])
			{
				pointsM += yo*5;
				ScoreEffect *s = new ScoreEffect(gameM->getFont(), xoffset+fieldWidth*16+5, yoffset+(yo)*16, yo*5);
				gameM->specialEffects.addEffect(s);
				return;
			}
		}

		Render(false);	// don't render current brick
		SDL_Rect r;
		NjamSetRect(r, xoffset, yoffset, 16*fieldWidth, 16*yo+16);
		SDL_FillRect(Screen, &r, color);

		char buffer[20];
		sprintf(buffer, "%3d", yo*5+5);
		gameM->getFont()->WriteTextXY(Screen, xoffset+fieldWidth*16-25, yoffset+yo*16, buffer);
		SDL_Flip(Screen);
		SDL_Delay(70);
	}
}
//-----------------------------------------------------------------------------
//! render field & current brick
void Player::Render(bool current)
{
	// render playing box border
	SDL_Rect r;
	Uint32 yellow = SDL_MapRGB(Screen->format, 255, 255, 0);
	NjamSetRect(r, xoffset-1, yoffset-1, 146, 370);
    SDL_FillRect(Screen, &r, yellow);

	// render fields
	for (int x = 0; x < fieldWidth; ++x)
	{
		for (int y = 0; y < fieldHeight; ++y)
		{
			int field = (int)fieldM[x][y];
            Uint32 FillColor = Shape::getColor(field);
			if (field == 0)
				FillColor = 0;
            SDL_Rect re;
            NjamSetRect(re, xoffset + x*16, yoffset+y*16, 16, 16);
            SDL_FillRect(Screen, &re, FillColor);
		}
	}

	std::string dropMarker("LINE");
	config().getValue("drop_marker", dropMarker);

	// render falling brick
	if (current)
	{
		if (dropMarker == "SHADOW")		// render where would it land if dropped
		{
			int yco = ycoordM;
			while (CheckPieceMove(xcoordM, yco+1))
				yco++;

			for (int x = 0; x < 4; ++x)
			{
				for (int y = 0; y < 4; ++y)
				{
					int cell = getCell(x, y, currentShapeM);
					if (cell == 0)
						continue;
					Uint32 FillColor = SDL_MapRGB(Screen->format, 50, 50, 50);;
					SDL_Rect re;
					NjamSetRect(re, xoffset + (xcoordM+x)*16, yoffset + (yco+y)*16, 16, 16);
					SDL_FillRect(Screen, &re, FillColor);
				}
			}
		}

		for (int x = 0; x < 4; ++x)
		{
			bool has_it = false;
			for (int y = 0; y < 4; ++y)
			{
				int cell = getCell(x, y, currentShapeM);
				if (cell == 0)
					continue;
				has_it = true;
				Uint32 FillColor = Shape::getColor(cell);
				SDL_Rect re;
				NjamSetRect(re, xoffset + (xcoordM+x)*16, yoffset + (ycoordM+y)*16, 16, 16);
				SDL_FillRect(Screen, &re, FillColor);
			}

			if (has_it && dropMarker == "LINE")		// render drop-spot
			{
				int y = ycoordM+2;
				while (y < fieldHeight && fieldM[xcoordM+x][y] == 0)
					y++;
				Uint32 FillColor = SDL_MapRGB(Screen->format, 255, 255, 0);
				SDL_Rect re;
				NjamSetRect(re, xoffset + (xcoordM+x)*16, yoffset + y*16 - 2, 16, 2);
				SDL_FillRect(Screen, &re, FillColor);
			}
		}
	}

	NjamSetRect(r, xstats-1, ystats-1, 120, 135);			// render points,level, next brick
    SDL_FillRect(Screen, &r, yellow);
	NjamSetRect(r, xstats, ystats, 118, 133);
    SDL_FillRect(Screen, &r, 0);
	char buffer[30];
	sprintf(buffer, "POINTS:%06d", pointsM);
	gameM->getFont()->WriteTextXY(Screen, xstats+5, ystats+5, buffer);
	sprintf(buffer, "LINES: %06d", rowsM);
	gameM->getFont()->WriteTextXY(Screen, xstats+5, ystats+20, buffer);

	gameM->getFont()->WriteTextXY(Screen, xstats+42, ystats+40, "NEXT");

	int gameType = 0;
	config().getValue("game_type", gameType);
	if (gameType == 2)	// bastet
	{
		gameM->getFont()->WriteTextXY(Screen, xstats+48, ystats+70, "N/A");
	}
	else
	{
		Shape s(nextShapeM, 0);
		for (int x = 0; x < 4; ++x)
		{
			for (int y = 0; y < 4; ++y)
			{
				int cell = s.getCell(x, y);
				if (cell == 0)
					continue;
				Uint32 FillColor = Shape::getColor(cell);
				SDL_Rect re;
				NjamSetRect(re, xstats+25 + x*16, ystats+60 + y*16, 16, 16);
				SDL_FillRect(Screen, &re, FillColor);
			}
		}
	}
}
//-----------------------------------------------------------------------------
void Player::AddBottomLines(int lines)
{
	for (int y = 0; y<fieldHeight-lines; ++y)				// move heap up
		for (int x=0; x<fieldWidth; ++x)
			fieldM[x][y] = fieldM[x][y+lines];

	for (int y = fieldHeight-lines; y < fieldHeight; ++y)
	{
		for (int x = 0; x<fieldWidth; ++x)					// add row of random cells
		{
			fieldM[x][y] = NjamRandom(7);
			fieldM[NjamRandom(fieldWidth)][y] = 0;			// make sure it has at least one hole
		}
	}
}
//-----------------------------------------------------------------------------
void Player::move(int x, int y)		// move current brick
{
	if (CheckPieceMove(xcoordM+x, ycoordM+y))
	{
		xcoordM += x;
		ycoordM += y;
	}
}
//-----------------------------------------------------------------------------
void Player::rotate(int spin)		// rotate current brick
{
	int oldrot = rotationM;
	rotationM += spin;
	if (rotationM < 0)
		rotationM = 3;
	if (rotationM > 3)
		rotationM = 0;
	if (CheckPieceRotate(xcoordM, ycoordM))
		countdown++;							// halt the falling so player can manage to rotate
	else
		rotationM = oldrot;						// cannot be rotated
}
//-----------------------------------------------------------------------------
bool Player::drop()					// drop current brick
{
	while (CheckPieceMove(xcoordM, ycoordM+1))
		ycoordM++;
	return Gravity(true);
}
//-----------------------------------------------------------------------------
// returns what would be the worst brick for that player
// uses "Bastet" algorythm: http://fph.altervista.org/prog/bastet.shtml
//
// 1. drop the shape in every possible position and rotation
// 2. calc the score for that position
//      check for full lines, give 2000 to each, and remove the full lines
//      calc the height of each column: score += 2*col_height
// 3. add +/- 2 to each score
// 4. sort the shapes by their score
// 5. gives the worst shape (with some % of chance) = 75% worst, 92% second worst, 98%, 100% for others
//
int Player::getWorstBrick()
{
	char tempField[fieldWidth][fieldHeight];

	int brickScores[7];				// score for each brick
	for (int myshape = 0; myshape < 7; myshape++)
	{
		brickScores[myshape] = -30000;
		for (int xc=-2; xc<fieldWidth+2; xc++)
		{
			for (int rot=0; rot<4; rot++)
			{
				for (int xx=0; xx<fieldWidth; ++xx)				// copy
					for (int yy=0; yy<fieldHeight; ++yy)
						tempField[xx][yy] = fieldM[xx][yy];

				int yc = 0;
				bool found = false;		// drop down
				while (!found && yc < fieldHeight)
				{
					yc++;
					for (int x=0; x<4 && !found; x++)
					{
						int xxc = xc+x;
						if (xxc >= fieldWidth || xxc < 0)
							continue;
						for (int y=0; y<4 && !found; y++)
						{
							if (!getCell(x, y, myshape, rot))
								continue;

							int yyc = yc+y;
							if (yyc >= fieldHeight || tempField[xxc][yyc])
							{
								found = true;
								break;
							}
						}
					}
				}

				if (!found && yc >= fieldHeight)	// cannot place at all
					continue;

				yc--;
				for (int xx=0; xx<4; xx++)					// place the brick on the heap
				{
					for (int yy=0; yy<4; yy++)
					{
						char c = getCell(xx,yy,myshape, rot);
						if (c)
						{
							if (xc+xx < 0 || xc+xx >= fieldWidth || yc+yy >= fieldHeight)
								continue;

							tempField[xc+xx][yc+yy] = c;
						}
					}
				}

				int score = 2000 * CheckLine(&tempField);
				for (int xx=0; xx<fieldWidth; ++xx)
				{
					for (int yy=0; yy<fieldHeight; ++yy)
					{
						if (tempField[xx][yy])
						{
							score -= 2*(fieldHeight - yy);
							break;
						}
					}
				}

				if (score > brickScores[myshape])
					brickScores[myshape] = score;
			}
		}
	}

	int bs[7];
	for (int i=0; i<7; i++)
	{
		brickScores[i] += NjamRandom(5) - 2;
		bs[i] = i;
	}

	// sort
	for (int i=0; i<7; i++)
	{
		for (int j=i; j<7; j++)
		{
			if (brickScores[i] > brickScores[j])
			{
				int t = brickScores[i];
				brickScores[i] = brickScores[j];
				brickScores[j] = t;
				t = bs[i];
				bs[i] = bs[j];
				bs[j] = t;
			}
		}
	}

	#ifdef DEBUG_BASTET
	for (int i=0; i<7; i++)
		printf("%d. SCORE: shape = %d, score = %d\n", i, bs[i], brickScores[i]);
	#endif

	int shape;
	const int chances[7] = { 75, 92, 98, 100, 100, 100, 100 };
	for (int i=0; i<7; ++i)
		if (NjamRandom(100) < chances[i])
			return bs[i];

	// should never happen
	return bs[0];
}
//-----------------------------------------------------------------------------
