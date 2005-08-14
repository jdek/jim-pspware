#ifndef AB_HISCORE_H
#define AB_HISCORE_H
#include <string>
#include <list>
//-----------------------------------------------------------------------------
class HiScore
{
public:
	std::string name;
	int level;
	int points;
	bool operator<(const HiScore& h) { return points > h.points; };
};
//-----------------------------------------------------------------------------
class HiScores
{
	std::list<HiScore> list;
public:
	typedef std::list<HiScore>::iterator iterator;
	void addHiscore(std::string name, int level, int points);
	bool canEnter(int score);

	iterator begin() { return list.begin(); };
	iterator end()   { return list.end(); };
	HiScores();
	~HiScores();

private:
	HiScores(const HiScores &);		// disallow copy
};
//-----------------------------------------------------------------------------
HiScores &hiScores();
//-----------------------------------------------------------------------------
#endif
