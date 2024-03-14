#include <errno.h>
#include <string.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

#include "ansi.h"

void help(char* program_name)
{
	printf("\
Expected:\n\
%s "UNDERLINE"PID"RESET_UNDERLINE"\n\
", program_name);
}

void exit_error(int error_code)
{
	printf(BOLD RED"Error:"RESET_BOLD WHITE" %s\n", strerror(error_code));
	exit(error_code);
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		help(argv[0]);
		return (0);
	}

	int tracee_pid = atoi(argv[1]);

	if (ptrace(PTRACE_ATTACH, tracee_pid, 0, 0) == -1)
		exit_error(errno);

	if (waitpid(tracee_pid, NULL, 0) == -1)
		exit_error(errno);

	struct user_regs_struct regs;
	while (true)
	{
		if (ptrace(PTRACE_SINGLESTEP, tracee_pid, 0, 0) == -1)
			exit_error(errno);

		if (waitpid(tracee_pid, NULL, 0) == -1)
			break;

		if (ptrace(PTRACE_GETREGS, tracee_pid, 0, &regs) == -1)
			break;
		printf("size of stack: %llx\n", regs.rbp - regs.rsp);
		printf("rax: %llx\n", regs.rax);
	}
	if (ptrace(PTRACE_DETACH, tracee_pid, 0, 0) == -1)
		exit_error(errno);
	return (0);
}