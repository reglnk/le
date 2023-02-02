#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <CB/cb.h>

// getArr return codes
#define GA_FOUND         0x01
#define GA_PARAM_FOUND   0x02

// processDirectory return codes
#define PD_UNDEFINED_ENTRY 0x02
#define PD_MEM_ERROR       0x04
#define PD_STAT_ERROR      0x02

int getStatInfo (
	unsigned int raw,
	char *const special,
	char *const modes
) {
	unsigned entry_type = raw & S_IFMT;
	unsigned index = 0;

	special[index++] = (raw & S_ISUID) ? 'u' : '_';
	special[index++] = (raw & S_ISGID) ? 'g' : '_';
	special[index++] = (raw & S_ISVTX) ? 'v' : '_';

	special[index] = '\0';
	index = 0;

	modes[index++] = (raw & S_IRUSR) ? 'r' : '_';
	modes[index++] = (raw & S_IWUSR) ? 'w' : '_';
	modes[index++] = (raw & S_IXUSR) ? 'x' : '_';
	modes[index++] = (raw & S_IRGRP) ? 'r' : '_';
	modes[index++] = (raw & S_IWGRP) ? 'w' : '_';
	modes[index++] = (raw & S_IXGRP) ? 'x' : '_';
	modes[index++] = (raw & S_IROTH) ? 'r' : '_';
	modes[index++] = (raw & S_IWOTH) ? 'w' : '_';
	modes[index++] = (raw & S_IXOTH) ? 'x' : '_';

	modes[index] = '\0';
	return 0;
}

int processDirectory (
	const char *dirname,
	int showHidden,
	int allowRecToHidden,
	int curRecursionLvl, // used to offset nested entries
	int maxRecursionLvl,
	int nameWidth
) {
	struct dirent **entries = NULL;

	int nument = scandir(dirname, &entries, NULL, alphasort);

	if (nument == -1) {
		if (entries)
			free(entries);
		return 0; // just a read-only directory
	}

	char *namebuffer = malloc(1024);
	if (!namebuffer) {
		free(entries);
		return PD_MEM_ERROR;
	}

	size_t namelength = strlen(dirname);
	memcpy(namebuffer, dirname, namelength);
	if (namelength >= 1 && namebuffer[namelength - 1] != '/')
		namebuffer[namelength++] = '/';

	int retcode = 0;

	for (int e = 0; e < nument; ++e)
	{
		struct dirent *ce = entries[e];
		unsigned char dt = ce->d_type;
		strcpy(namebuffer + namelength, ce->d_name);

		const char *ntype =
			dt == DT_REG ?     "regular" :
			dt == DT_LNK ?     "symlink" :
			dt == DT_BLK ?     "blk dev" : // block device
			dt == DT_DIR ?     "<dir>  " :
			dt == DT_SOCK ?    "socket " :
			dt == DT_CHR ?     "chr dev" : // char device
			dt == DT_FIFO ?    "FIFO   " : // a named pipe
			dt == DT_WHT ?     "whtout " : // 'whiteout' from BSD
			dt == DT_UNKNOWN ? "unknown" :
								"!error!";

		if (ntype[0] == '!')
			retcode |= PD_UNDEFINED_ENTRY;

		const char *end1 = dt == DT_DIR ? "/" : dt == DT_LNK ? "&" : " ";
		const int isCpDir = !strcmp(ce->d_name, ".") || !strcmp(ce->d_name, "..");
		const int toBeHidden = !showHidden && ce->d_name[0] == '.';

		struct stat st;
		if (stat(namebuffer, &st) != 0)
			retcode |= PD_STAT_ERROR;

		if (!isCpDir && !toBeHidden)
		{
			char spec[4];
			char modes[10];
			getStatInfo(st.st_mode, spec, modes);

			int charsLeft = nameWidth - 4 * curRecursionLvl;
			if (charsLeft < 0)
				charsLeft = 0;

			cbPushBuffer();

			cbColorv("0;37"); // light grey
			cbprintf("[%s | %s | %s]", ntype, modes, spec);

			cbColorv("1;30"); // dark grey bold foreground
			for (int i = curRecursionLvl; i; --i)
				cbprintf(" ___");

			if (dt == DT_DIR)
				cbColorv("1;34");
			else if (dt == DT_LNK)
				cbColorv("0;36");
			else if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
				cbColorv("0;32"); // green
			else cbColorv("0;0");

			cbprintf(" %-*s", charsLeft, ce->d_name);

			cbColorv("0;0");
			cbprintf("%s | ino: %0lu off: %0lu rl: %u\n", end1, ce->d_ino, ce->d_off, ce->d_reclen);

			cbPopBuffer();
		}

		if (curRecursionLvl < maxRecursionLvl && dt == DT_DIR && !isCpDir && (allowRecToHidden || !toBeHidden)) {
			retcode |= processDirectory(namebuffer, showHidden, allowRecToHidden, curRecursionLvl + 1, maxRecursionLvl, nameWidth);
		}
	}
	free(entries);
	free(namebuffer);
	
	return 0;
}

int getArg(int argc, char **argv, const char *key, char **dest)
{
	int retcode = 0;

	for (int i = 1; i < argc; ++i) {
		char *eq = strchr(argv[i], '=');
		if (eq)
			*eq = '\0';
		if (!strcmp(argv[i], key)) {
			retcode |= GA_FOUND;
			if (eq && dest) {
				retcode |= GA_PARAM_FOUND;
				size_t keylen = strlen(eq + 1);
				*dest = eq + 1;
			}
		}
		if (eq)
			*eq = '=';
	}
	return retcode;
}

int getNextArg(int argc, char **argv, int prev) {
	for (int i = prev; i < argc; ++i)
		if (argv[i][0] != '-')
			return i;
	return -1;
}

int main(int argc, char **argv)
{
	int showHidden = 1;
	int recursionLevel = 0;
	int allowRecToHidden = 0;
	int nameWidth = 40;

	if (getArg(argc, argv, "--help", NULL) & GA_FOUND) {
		printf(
			"Usage: %s <path>\n"
			"    opt=value: desc      (default behaviour, the value if omitted)\n"
			"\n"
			"    -s={0,1}: show hidden files if passed with 1       (show,   1)\n"
			"    -d={int}: set recursion depth                      (   0,  16)\n"
			"    -r={0,1}: allow recursion to hidden directories    (  no,   1)\n"
			"    -w={int}: set entry name width                     (  40, 100)\n",
			argv[0]);
		return 0;
	}

	int argfound;
	char *arg1;

	argfound = getArg(argc, argv, "-s", &arg1);
	if (argfound & GA_PARAM_FOUND) {
		showHidden = atoi(arg1);
	} else if (argfound & GA_FOUND)
		showHidden = 1;

	argfound = getArg(argc, argv, "-d", &arg1);
	if (argfound & GA_PARAM_FOUND) {
		recursionLevel = atoi(arg1);
	} else if (argfound & GA_FOUND)
		recursionLevel = 16;

	argfound = getArg(argc, argv, "-r", &arg1);
	if (argfound & GA_PARAM_FOUND) {
		allowRecToHidden = atoi(arg1);
	} else if (argfound & GA_FOUND)
		allowRecToHidden = 1;

	argfound = getArg(argc, argv, "-w", &arg1);
	if (argfound & GA_PARAM_FOUND) {
		nameWidth = atoi(arg1);
	} else if (argfound & GA_FOUND)
		nameWidth = 100;

	const char *dirname = ".";
	argfound = getNextArg(argc, argv, 1);
	if (argfound != -1)
		dirname = argv[argfound];

	return processDirectory(dirname, showHidden, allowRecToHidden, 0, recursionLevel, nameWidth);
}
