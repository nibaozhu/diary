

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <stdlib.h>


int main(int argc, char **argv)
{
	
	while (1) {
		static i = 0;
		const char *prompt = "\r\n(myshell) ";
		if (i == 0) {
			prompt = "Welcome ...\r\n(myshell) ";
			i++;
		}
		char *command = readline (prompt);
		printf("[%s]", command);
		free(command);
	}

	return 0;
}
