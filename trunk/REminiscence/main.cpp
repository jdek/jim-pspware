/* REminiscence - Flashback interpreter
 * Copyright (C) 2005 Gregory Montoir
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "game.h"
#include "resource.h"
#include "systemstub.h"

static const char *USAGE = 
	"REminiscence - Flashback Interpreter\n"
	"Usage: rs [OPTIONS]...\n"
	"  --datapath=PATH   Path to where the game is installed (default '.')\n"
	"  --savepath=PATH   Path to where the save files are stored (default '.')\n"
	"  --version=VER     Version of the game to load : fr, sp, de, us (default)\n";

static const struct {
	const char *name;
	Version ver;
} VERSIONS[] = {
	{ "fr",  VER_FR },
	{ "sp",  VER_SP },
	{ "de",  VER_DE },
	{ "us",  VER_US }
};

static bool parseOption(const char *arg, const char *longCmd, const char **opt) {
	bool ret = false;
	if (arg[0] == '-' && arg[1] == '-') {
		if (strncmp(arg + 2, longCmd, strlen(longCmd)) == 0) {
			*opt = arg + 2 + strlen(longCmd);
			ret = true;
		}
	}
	return ret;
}

#ifdef PSP
/* Use the path to EBOOT.PBP to determine the current directory. */
char * psp_getcwd(char *eboot_path)
{
	static char psp_full_path[1024 + 1];
	char *psp_eboot_path;

	strncpy(psp_full_path, eboot_path, sizeof(psp_full_path) - 1);
	psp_full_path[sizeof(psp_full_path) - 1] = '\0';
	psp_eboot_path = strrchr(psp_full_path, '/');
	if (psp_eboot_path != NULL) {
		*psp_eboot_path = '\0';
	}

	return psp_full_path;
}
#endif

#ifndef PSP
#undef main
#else
/* On the PSP we need main defined as SDL_main. */
extern "C"
#endif
int main(int argc, char *argv[]) {
#ifndef PSP
	const char *dataPath = ".";
	const char *savePath = ".";
#else
	char _dataPath[1024 + 1];
	char _savePath[1024 + 1];

	strncpy(_dataPath, psp_getcwd(argv[0]), sizeof(_dataPath) - 1);
	strncat(_dataPath, "/data", sizeof(_dataPath) - 1);
	_dataPath[sizeof(_dataPath) - 1] = '\0';
	strncpy(_savePath, psp_getcwd(argv[0]), sizeof(_savePath) - 1);
	strncat(_savePath, "/data", sizeof(_savePath) - 1);
	_savePath[sizeof(_savePath) - 1] = '\0';

	const char *dataPath = (const char *) _dataPath;
	const char *savePath = (const char *) _savePath;
#endif
	const char *version = 0;
	Version ver = VER_US;
	for (int i = 1; i < argc; ++i) {
		bool opt = false;
		if (strlen(argv[i]) >= 2) {
			opt |= parseOption(argv[i], "datapath=", &dataPath);
			opt |= parseOption(argv[i], "savepath=", &savePath);
			opt |= parseOption(argv[i], "version=", &version);
		}
		if (!opt) {
			printf(USAGE);
			return 0;
		}
	}
	if (version) {
		for (unsigned int j = 0; j < ARRAYSIZE(VERSIONS); ++j) {
			if (strcmp(version, VERSIONS[j].name) == 0) {
				ver = VERSIONS[j].ver;
				break;
			}
		}
	}
	g_debugMask = DBG_INFO; // DBG_CUT | DBG_VIDEO | DBG_RES | DBG_MENU | DBG_PGE | DBG_GAME | DBG_UNPACK | DBG_COL | DBG_MOD;
	SystemStub *stub = SystemStub_SDL_create();
	Game *g = new Game(stub, dataPath, savePath, ver);
	g->run();
	delete g;
	delete stub;
	return 0;
}
