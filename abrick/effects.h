#ifndef AB_EFFECTS_H
#define AB_EFFECTS_H

#include <list>
//-----------------------------------------------------------------------------
class Game;
//-----------------------------------------------------------------------------
class SpecialEffect
{
protected:
	NjamFont *fontM;

public:
	SpecialEffect(NjamFont *f) { fontM = f; };
	virtual bool Update() = 0;
};
//-----------------------------------------------------------------------------
class Effects
{
private:
	std::list<SpecialEffect *> listM;
public:
	void addEffect(SpecialEffect *);
	void removeEffect(SpecialEffect *);
	void Render();

	~Effects();
};
//-----------------------------------------------------------------------------
class ScoreEffect: public SpecialEffect
{
private:
	int counterM, xposM, yposM, pointsM;
public:
	ScoreEffect(NjamFont *f, int xpos, int ypos, int points)
		: SpecialEffect(f), counterM(0), xposM(xpos), yposM(ypos), pointsM(points) {};
	virtual bool Update();
};
//-----------------------------------------------------------------------------
class TextEffect: public SpecialEffect
{
private:
	int repeatM;
	int counterM, xposM, yposM;
	std::string textM;
public:
	TextEffect(NjamFont *f, int xpos, int ypos, const std::string& text, int repeat = 0)
		: SpecialEffect(f), counterM(0), xposM(xpos), yposM(ypos), textM(text), repeatM(repeat) {};
	virtual bool Update();
};
//-----------------------------------------------------------------------------
class ScrollEffect: public SpecialEffect
{
private:
	double xoffset;
	int counterM, xposM, yposM;
	std::string textM;
public:
	ScrollEffect(NjamFont *f, int xpos, int ypos, const std::string& text)
		: SpecialEffect(f), counterM(0), xposM(xpos), yposM(ypos), textM(text), xoffset(640) {};
	virtual bool Update();
};
//-----------------------------------------------------------------------------
#endif


