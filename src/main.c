#include <errno.h>
#include <string.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

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

	struct user_regs_struct	regs;
	// long					data;
	while (true)
	{
		if (ptrace(PTRACE_SINGLESTEP, tracee_pid, 0, 0) == -1)
			exit_error(errno);

		if (waitpid(tracee_pid, NULL, 0) == -1)
			break;

		if (ptrace(PTRACE_GETREGS, tracee_pid, 0, &regs) == -1)
			break;

		// data = ptrace(PTRACE_PEEKDATA, tracee_pid, 0, regs.rsp);
		// printf("rbp: %llx, rsp: %llx\n", regs.rbp, regs.rsp);
		// printf("size of stack: %lli\n", regs.rbp - regs.rsp);
		// write(1, "stack top: ", 11);
		// write(1, &data, 8);
		// write(1, "\n", 1);
		printf(BOLD" -- STEP: --\n"RESET_BOLD);
		printf(CYAN"rax: %llx\n"WHITE, regs.rax);
		printf(BLUE"rdi: %llx\n"WHITE, regs.rdi);
		printf(CYAN"rsi: %llx\n"WHITE, regs.rsi);
		printf(BLUE"rdx: %llx\n"WHITE, regs.rdx);
		printf(CYAN"rcx: %llx\n"WHITE, regs.rcx);
	}
	if (ptrace(PTRACE_DETACH, tracee_pid, 0, 0) == -1)
		exit_error(errno);
	return (0);
}