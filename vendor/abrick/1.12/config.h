//! Class that handles reading/writing of configuration options
//! and provides interface to all interested parties
//
#ifndef AB_CONFIG_H
#define AB_CONFIG_H

#include <map>
#include <vector>
#include <string>
//---------------------------------------------------------------------------------------
//! Do not instantiate objects of this class. Use config() function (see below).
class Config
{
public:
	bool save();
	bool load();

	// return false if value exists, false if not
	bool getValue(std::string key, std::string& value);
	bool getValue(std::string key, int& value);
	bool getValue(std::string key, double& value);
	bool getValue(std::string key, bool& value);

	bool getValue(std::string key);

	// return false if value existed, false if not. Pass false in saveIt
	// to prevent saving the config file before setValue() returns (f. ex.
	// when a number of items are being stored at one time).
	bool setValue(std::string key, std::string value, bool saveIt = false);
	bool setValue(std::string key, int value, bool saveIt = false);
	bool setValue(std::string key, double value, bool saveIt = false);
	bool setValue(std::string key, bool value, bool saveIt = false);

	Config();
	~Config();
private:
	std::map<std::string, std::string> dataM;		//! key -> value
};

Config& config();
//---------------------------------------------------------------------------------------
#endif
