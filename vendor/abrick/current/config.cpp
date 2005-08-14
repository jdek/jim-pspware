//------------------------------------------------------------------------------
#include <string>
#include <fstream>
#include <sstream>
#include "config.h"
//------------------------------------------------------------------------------
using namespace std;
//-----------------------------------------------------------------------------
Config& config()
{
	static Config c;
	return c;
}
//-----------------------------------------------------------------------------
//! return true if value exists, false if not
bool Config::getValue(string key, string& value)
{
	if (dataM.find(key) == dataM.end())
		return false;

	value = dataM[key];
	return true;
}
//-----------------------------------------------------------------------------
bool Config::getValue(std::string key)
{
	bool b;
	if (getValue(key, b))
		return b;
	else
		return true;	// all options are ON by default
}
//-----------------------------------------------------------------------------
bool Config::getValue(string key, int& value)
{
	string s;
	if (!getValue(key, s))
		return false;

	stringstream ss;
	ss << s;
	ss >> value;
	return true;
}
//-----------------------------------------------------------------------------
bool Config::getValue(string key, double& value)
{
	string s;
	if (!getValue(key, s))
		return false;

	stringstream ss;
	ss << s;
	ss >> value;
	return true;
}
//-----------------------------------------------------------------------------
bool Config::getValue(string key, bool& value)
{
	string s;
	if (!getValue(key, s))
		return false;

	value = (s == "1");
	return true;
}
//-----------------------------------------------------------------------------
//! return true if value existed, false if not
bool Config::setValue(string key, string value, bool saveIt)
{
	bool ret = (dataM.end() != dataM.find(key));
	if (ret)
		dataM.erase(key);
	dataM[key] = value;
	if (saveIt)
		save();
	return ret;
}
//-----------------------------------------------------------------------------
bool Config::setValue(string key, int value, bool saveIt)
{
	stringstream ss;
	ss << value;
	return setValue(key, ss.str(), saveIt);
}
//-----------------------------------------------------------------------------
bool Config::setValue(string key, double value, bool saveIt)
{
	stringstream ss;
	ss << value;
	return setValue(key, ss.str(), saveIt);
}
//-----------------------------------------------------------------------------
bool Config::setValue(string key, bool value, bool saveIt)
{
	if (value)
		return setValue(key, string("1"), saveIt);
	else
		return setValue(key, string("0"), saveIt);
}
//-----------------------------------------------------------------------------
Config::Config()
{
	load();

	// setup default keys if not available
	char names[10][15] = {
		"PLAYER1 DOWN  ",		"PLAYER1 DROP  ",		"PLAYER1 LEFT  ",		"PLAYER1 RIGHT ",	"PLAYER1 ROTATE",
		"PLAYER2 DOWN  ",		"PLAYER2 DROP  ",		"PLAYER2 LEFT  ",		"PLAYER2 RIGHT ",	"PLAYER2 ROTATE"
	};

	int keys[10] = { 274, 32, 276, 275, 273, 115, 113, 97, 100, 119 };
	for (int i=0; i<10; ++i)
		if (dataM.find(names[i]) == dataM.end())
			setValue(names[i], keys[i]);
}
//-----------------------------------------------------------------------------
Config::~Config()
{
	save();
}
//-----------------------------------------------------------------------------
bool Config::save()
{
	std::ofstream file("ab.conf");
	if (!file)
		return false;

	file << "Abandoned Bricks configuration file." << endl << endl << "[Settings]" << endl;
	for (map<string, string>::const_iterator it = dataM.begin(); it != dataM.end(); ++it)
	{
		file << (*it).first << "=" << (*it).second << endl;
	}
	file.close();
	return true;
}
//-----------------------------------------------------------------------------
// this gets called from main() so we're sure config.ini is in the right place
bool Config::load()
{
	std::ifstream file("ab.conf");
	if (!file)
		return false;

	// I had to do it this way, since standard line << file, doesn't work good if data has spaces in it.
	std::stringstream ss;		// read entire file into string buffer
	ss << file.rdbuf();
	std::string s(ss.str());

	dataM.clear();
	while (true)
	{
		string::size_type t = s.find('\n');
		if (t == string::npos)
			break;

		string line = s.substr(0, t);
		s.erase(0, t+1);

		string::size_type p = line.find('=');
		if (p == string::npos)
			continue;

		string key = line.substr(0, p);
		line.erase(0, p + 1);
		line.erase(line.find_last_not_of(" \t\n\r")+1);	// right trim

		setValue(key, line, false);
	}

	file.close();
	return true;
}
//-----------------------------------------------------------------------------
