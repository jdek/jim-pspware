#ifndef AB_PLAYER_H
#define AB_PLAYER_H
//-----------------------------------------------------------------------------
class Game;
//-----------------------------------------------------------------------------
class Player
{
private:
	bool canPlayM;
	int speed_coef;
	int countdown;

	int xoffset, yoffset, xstats, ystats;
	Game *gameM;

	static const int fieldWidth = 9;
	static const int fieldHeight = 23;
	char fieldM[fieldWidth][fieldHeight];
	int currentShapeM, nextShapeM, rotationM;
	int pointsM, rowsM;
	int xcoordM, ycoordM;

	bool CheckPieceMove(int x, int y);
	bool CheckPieceRotate(int& x, int& y);
	bool NextBrick();
	int CheckLine(char (*field)[fieldWidth][fieldHeight] = 0);
	char getCell(int x, int y, int shape, int rotation = -1);

public:
	Player() { currentShapeM = nextShapeM = 0; gameM = 0; };
	void SetupGame(Game *game, int x, int y, int xs, int ys);
	void SetupLevel();
	void Render(bool current=true);
	bool Gravity(bool force=false);

	void LevelComplete();		// render nice animation
	void AddBottomLines(int lines);
	void move(int x, int y);	// move current brick
	void rotate(int spin = 1);	// rotate current brick
	bool drop();				// drop current brick

	int getScore() { return pointsM; };
	bool canPlay() { return canPlayM; };

	int getWorstBrick();		// "bastet" mode
};
//-----------------------------------------------------------------------------
#endif
