#include <stdio.h>

#include "ansi.h"

void help(char* program_name)
{
	printf("\
Expected:\n\
%s "UNDERLINE"PID"RESET_UNDERLINE"\n\
", program_name);
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		help(argv[0]);
		return (0);
	}
	return (0);
}