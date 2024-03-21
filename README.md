# Examini
Examini is a small debugging tool for examining the machine code ran by a linux process.

## Features
Currently Examini allows the user to either start a process as a child or to trace an external process by passing its pid. It then shows a disassembled assembly code in the terminal, this can be done either continuously or step by step.

As of now Examini only supports 64-bit machine code.

## Using
Below is an example of using Examini:
```bash
./examini echo "test"
	mov rdi, rsp
	call 0x0000000000000BFD
	push rbp
	lea rsi, [0xFFFFFFFFFFFE49CF]
	lea rax, [0x0000000000017CA0]
	movq xmm1, rsi
	movq xmm2, rax
	punpcklqdq xmm1, xmm2
	mov rbp, rsp
	...
```
or by using the `-c PID` option to examine an already running process.

Run `./examini -h` for more options.
### Building
Building is as simple as running make:
```bash
git clone --recursive https://github.com/Chimplement/examini.git examini
cd examini
make
```

## Why
I originally made this tool because I was experimenting with the ptrace syscall on Unix. At that time I just learned assembly so I was interested in looking at the instructions other programs execute, which is how this tool came to be.

## Future
Here are couple of features I might add in the near future:
- monitoring registers
- modifying registers