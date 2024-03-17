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

	// struct user_regs_struct		old_regs;
	struct user_regs_struct			regs;
	// bool							bold = false;
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

		// data = ptrace(PTRACE_PEEKDATA, tracee_pid, 0, regs.rsp);
		// printf("rbp: %llx, rsp: %llx\n", regs.rbp, regs.rsp);
		// printf("size of stack: %lli\n", regs.rbp - regs.rsp);
		// write(1, "stack top: ", 11);
		// write(1, &data, 8);
		// write(1, "\n", 1);
		// printf(BOLD" -- STEP: --\n"RESET_BOLD);
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
		// if (memcmp(&regs, &old_regs, sizeof(regs)))
		// {
		// 	if (bold)
		// 		printf(BOLD);
		// 	printf("rax: %-16llx\t", regs.rax);
		// 	printf("rdi: %-16llx\t", regs.rdi);
		// 	printf("rsi: %-16llx\t", regs.rsi);
		// 	printf("rdx: %-16llx\t", regs.rdx);
		// 	printf("rcx: %-16llx\t", regs.rcx);
		// 	if (bold)
		// 		printf(RESET_BOLD);
		// 	printf("\n");
		// 	bold = !bold;
		// }
		// old_regs = regs;
	}
	if (ptrace(PTRACE_DETACH, tracee_pid, 0, 0) == -1)
		exit_error(errno);
	return (0);
}