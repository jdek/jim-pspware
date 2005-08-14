#include "stdio.h"
#include "hiscore.h"
//-----------------------------------------------------------------------------
HiScores &hiScores()
{
	static HiScores h;
	return h;
}
//-----------------------------------------------------------------------------
void HiScores::addHiscore(std::string name, int level, int points)
{
	HiScore hs;
	hs.name = name;
	hs.level = level;
	hs.points = points;
	list.push_back(hs);
	list.sort();

	while (list.size() > 15)
		list.pop_back();
}
//-----------------------------------------------------------------------------
bool HiScores::canEnter(int score)
{
	return (score > list.back().points);
}
//-----------------------------------------------------------------------------
HiScores::HiScores()
{
	// on Linux use /usr/share/abandoned/hiscore.dat  ?
	FILE *fp = fopen("hiscore.dat", "r");
	if (!fp)
	{
		std::string defaultNames[] = {		// create default hiscore
			"TOMAS", 		"PAULO",		"LOUISE", 		"JULIAN",		"GEORGE",
			"ENZO", 		"JOLAN",		"JAAP",			"MARTIN", 		"RICHARD",
			"CLAUS",		"THORSTEN",		"ELLE",			"SABRINA",		"JOSHUA"
		};
		for (int i=0; i<15; ++i)
			addHiscore(defaultNames[i], (int)(15.0 - i*0.6), 10000 - i*500);
	}
	else
	{
		// load hiscore from file (if any), format: NAME#POINTS#LEVEL#
		char buff[80];
		int number = 0;
		while (!feof(fp) && number < 15)
		{
			fgets(buff, 80, fp);
			int i = 0, last;

			// name
			std::string name;
			while (buff[i] != '#' && i < 20)
			{
				name += buff[i];
				i++;
			}

			i++;
			last = i;
			while (buff[i] != '#' && i < 70)
				i++;
			if (i >= 70)
				break;
			buff[i] = '\0';
			int points = atoi(buff+last);
			i++;
			last = i;
			while (buff[i] != '#' && i < 70)
				i++;
			if (i >= 70)
				break;
			buff[i] = '\0';
			int level = atoi(buff+last);

			addHiscore(name, level, points);
			number++;
		}
		fclose(fp);
	}
}
//-----------------------------------------------------------------------------
HiScores::~HiScores()
{
	// save to file
	// format: NAME#POINTS#SWAPS#
	FILE *fp = fopen("hiscore.dat", "w+");
	if (fp)
	{
		for (iterator it = begin(); it != end(); ++it)
			fprintf(fp, "%s#%d#%d#\n", (*it).name.c_str(), (*it).points, (*it).level);
		fclose(fp);
	}
 	else
 		printf("Unable to write hiscore file!\n");
}
//-----------------------------------------------------------------------------
