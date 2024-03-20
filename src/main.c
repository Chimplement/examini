#define _GNU_SOURCE

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
#include "help.h"

void exit_error(int error_code)
{
	fprintf(stderr, BOLD RED"Error:"RESET_BOLD WHITE" %s\n", strerror(error_code));
	exit(error_code);
}

pid_t get_tracee(int argc, char* argv[], char* envp[])
{
	pid_t tracee;

	if (!strcmp(argv[0], "-p"))
	{
		if (argc != 3)
		{
			return (-2);
		}
		tracee = atoi(argv[1]);
		if (ptrace(PTRACE_ATTACH, tracee, 0, 0) == -1)
			return (-1);
	}
	else
	{
		tracee = fork();
		if (tracee == -1)
			return (-1);
		if (tracee == 0)
		{
			if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1)
				exit_error(errno);
			execvpe(argv[0], argv + 1, envp);
			exit_error(errno);
		}
	}
	return (tracee);
}

unsigned long get_ip(pid_t tracee)
{
	struct user_regs_struct regs;

	if (ptrace(PTRACE_GETREGS, tracee, 0, &regs) == -1)
		return (0);
	return (regs.rip);
}

int read_instruction(pid_t tracee, unsigned long ip, ZyanU8 instruction_buffer[sizeof(long)])
{
	long    instruction_long;

	errno = 0;
	instruction_long = ptrace(PTRACE_PEEKTEXT, tracee, ip, 0);
	if (errno != 0)
		return (-1);
	(void)memcpy(instruction_buffer, &instruction_long, sizeof(long));
	return (0);
}

int	print_instruction(ZyanU8 instruction_buffer[sizeof(long)])
{
	ZydisDisassembledInstruction instruction;

	if (ZYAN_FAILED(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, 0, instruction_buffer, sizeof(long), &instruction)))
		return (-1);
	fprintf(stderr, "%s", instruction.text);
	return (0);
}

void	wait_enter(void)
{
	int c;

	c = getchar();
	while(c != EOF && c != '\n')
		c = getchar();
}

int main(int argc, char* argv[], char* envp[])
{
	pid_t         tracee;
	int           status;
	unsigned long ip;
	ZyanU8        instruction_buffer[sizeof(long)]; 
	bool          single_step;
	int           arg_offset;

	if (argc < 2)
	{
		help(argv[0]);
		return (0);
	}
	if (!strcmp(argv[1], "-h"))
	{
		expanded_help(argv[0]);
		return (0);
	}

	arg_offset = 1;
	single_step = false;
	if (!strcmp(argv[1], "-s"))
	{
		arg_offset += 1;
		single_step = true;
	}

	tracee = get_tracee(argc - arg_offset, argv + arg_offset, envp);
	if (tracee == -1)
		exit_error(errno);
	if (tracee == -2)
	{
		help(argv[0]);
		return (0);
	}
	
	while (true)
	{
		if (waitpid(tracee, &status, 0) == -1)
			exit_error(errno);
		if (WIFEXITED(status))
			break;

		errno = 0;
		ip = get_ip(tracee);
		if (errno != 0)
			exit_error(errno);

		if (read_instruction(tracee, ip, instruction_buffer) == -1)
			exit_error(errno);
		
		(void)print_instruction(instruction_buffer);

		if (single_step)
		{
			fflush(stderr);
			wait_enter();
		}
		else
		{
			fprintf(stderr, "\n");
		}

		if (ptrace(PTRACE_SINGLESTEP, tracee, 0, 0) == -1)
			exit_error(errno);
	}
	(void)ptrace(PTRACE_DETACH, tracee, 0, 0);
	return (WEXITSTATUS(status));
}