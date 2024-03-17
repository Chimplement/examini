#include <stdio.h>

#include "ansi.h"

void help(char* program_name)
{
	printf("Expected:\n");
	printf("%s "UNDERLINE"COMMAND"RESET_UNDERLINE" ["UNDERLINE"ARGS"RESET_UNDERLINE"...]\n", program_name);
	printf("%s -p "UNDERLINE"PID"RESET_UNDERLINE"\n", program_name);
	printf("%s -h\n", program_name);
}

void expanded_help(char* program_name)
{
	printf("%s "UNDERLINE"COMMAND"RESET_UNDERLINE" ["UNDERLINE"ARGS"RESET_UNDERLINE"...]\n", program_name);
	printf("\tStarts "UNDERLINE"COMMAND"RESET_UNDERLINE" in a child process, showing every instruction it executes.\n");
	printf("\n");
	printf("%s -p "UNDERLINE"PID"RESET_UNDERLINE"\n", program_name);
	printf("\tStarts observing the process identified by "UNDERLINE"PID"RESET_UNDERLINE", showing every instruction it executes.\n");
	printf("\n");
	printf("%s -h\n", program_name);
	printf("\tShows this menu.\n");
	printf("\n");
}