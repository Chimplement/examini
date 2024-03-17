#include <errno.h>
#include <string.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

#include "Zydis/Disassembler.h"

#include "ansi.h"

void help(char* program_name)
{
	printf("Expected:\n");
	printf("%s "UNDERLINE"COMMAND"RESET_UNDERLINE" ["UNDERLINE"ARGS"RESET_UNDERLINE"...]\n", program_name);
	printf("%s -p "UNDERLINE"PID"RESET_UNDERLINE"\n", program_name);
}

void exit_error(int error_code)
{
	printf(BOLD RED"Error:"RESET_BOLD WHITE" %s\n", strerror(error_code));
	exit(error_code);
}

int main(int argc, char* argv[], char* envp[])
{
	if (argc < 2)
	{
		help(argv[0]);
		return (0);
	}

	int tracee_pid;
	if (!strcmp(argv[1], "-p"))
	{
		tracee_pid = atoi(argv[2]);
	}
	else
	{
		if (access(argv[1], X_OK) == -1)
			exit_error(errno);
		tracee_pid = fork();
		if (tracee_pid == -1)
			exit_error(errno);
		if (tracee_pid == 0)
		{
			execve(argv[1], argv + 1, envp);
			exit_error(errno);
		}
	}

	if (ptrace(PTRACE_ATTACH, tracee_pid, 0, 0) == -1)
		exit_error(errno);

	if (waitpid(tracee_pid, NULL, 0) == -1)
		exit_error(errno);

	struct user_regs_struct			regs;
	long							ip_long;
	ZyanU8							ip_buffer[sizeof(long)];
	ZydisDisassembledInstruction	instruction; 
	while (true)
	{
		if (ptrace(PTRACE_SINGLESTEP, tracee_pid, 0, 0) == -1)
			exit_error(errno);

		if (waitpid(tracee_pid, NULL, 0) == -1)
			break;

		if (ptrace(PTRACE_GETREGS, tracee_pid, 0, &regs) == -1)
			break;

		errno = 0;
		ip_long = ptrace(PTRACE_PEEKTEXT, tracee_pid, regs.rip, 0);
		if (errno != 0)
			break;
		memcpy(ip_buffer, &ip_long, sizeof(long));
		
		if (ZYAN_SUCCESS(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, 0, 
											ip_buffer, sizeof(ip_buffer), &instruction)))
		{
			printf("%s\n", instruction.text); 
		}
	}
	(void)ptrace(PTRACE_DETACH, tracee_pid, 0, 0);
	return (0);
}