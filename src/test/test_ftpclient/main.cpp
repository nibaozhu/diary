/***************************
 * author: nibz@qq.com
 * */


#include "ftplib.h"
#include <stdlib.h>
#include <linux/limits.h>
#include <string.h>
#include <iostream>

int main(int ac, char **av) {
	if (ac <= 2) {
		std::cerr << "Usage: " << av[0] << " inputFile remotePath" << std::endl;
		return 0;
	}

	int ret = 0;
	netbuf *nControl = NULL;
	const char *host = "0";

	const char *user = "nbz";
	const char *pass = "123";
	const char *TMP = ".tmp";

	int opt = FTPLIB_CONNMODE;
	long val = FTPLIB_PASSIVE;

	char x_path[PATH_MAX] = {0};
	int max = PATH_MAX;

	char tmpstr[PATH_MAX] = {0};
	strncpy(tmpstr, av[1], strlen(av[1]));
	strncpy(tmpstr + strlen(av[1]), TMP, strlen(TMP));
	const char *inputfile = av[1];
	const char *path = av[2];
	const char *tmpfile = tmpstr;
	char mode = FTPLIB_TEXT;

	const char *src = tmpfile;
	const char *dst = inputfile;

	do {
		/*
		 * FtpConnect - connect to remote server
		 *
		 * return 1 if connected, 0 if not
		 */
		ret = FtpConnect(host, &nControl);
		if (ret == 0) {
			std::cerr << "FtpConnect is failure!\n" << std::endl;
			break;
		}

		
		/*
		 * FtpLogin - log in to remote server
		 *
		 * return 1 if logged in, 0 otherwise
		 */
		ret = FtpLogin(user, pass, nControl);
		if (ret == 0) {
			std::cerr << "FtpLogin is failure!\n" << std::endl;
			break;
		}


		/*
		 * FtpOptions - change connection options
		 *
		 * returns 1 if successful, 0 on error
		 */
		ret = FtpOptions(opt, val, nControl);
		if (ret == 0) {
			std::cerr << "FtpLogin is failure!\n" << std::endl;
			break;
		}

		/*
		 * FtpPwd - get working directory at remote
		 *
		 * return 1 if successful, 0 otherwise
		 */
		ret = FtpPwd(x_path, max, nControl);
		if (ret == 0) {
			std::cerr << "FtpPwd is failure!\n" << std::endl;
			break;
		}

		/*
		 * FtpChdir - change path at remote
		 *
		 * return 1 if successful, 0 otherwise
		 */
		ret = FtpChdir(path, nControl);
		if (ret == 0) {
			std::cerr << "FtpChdir is failure!\n" << std::endl;
			break;
		}


		/*
		 * FtpPut - issue a PUT command and send data from input
		 *
		 * return 1 if successful, 0 otherwise
		 */
		ret = FtpPut(inputfile, tmpfile, mode, nControl);
		if (ret == 0) {
			std::cerr << "FtpPut is failure!\n" << std::endl;
			break;
		}


		/*
		 * FtpRename - rename a file at remote
		 *
		 * return 1 if successful, 0 otherwise
		 */
		ret = FtpRename(src, dst, nControl);
		if (ret == 0) {
			std::cerr << "FtpRename is failure!\n" << std::endl;
			break;
		}

	} while (0);

	/*
	 * FtpQuit - disconnect from remote
	 *
	 * return 1 if successful, 0 otherwise
	 */
	FtpQuit(nControl);

	return 0;
}
