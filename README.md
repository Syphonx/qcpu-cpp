<div align="center">
  <img width="888" height="150" src="./github_header.png">
</div>

# qcpu

## a fictional cpu implementation, with assembler and web  interface

**qcpu** is a fictional 16-bit CPU emulator written in node.js. It is inspired by the DCPU16 specification for the scrapped Mojang game [0x10c](https://en.wikipedia.org/wiki/0x10c), as well as [Zachtronics](http://www.zachtronics.com/)' assembly programming games. This repo contains scripts for running binary files (with optional terminal-based visualiser), as well as an assembler for converting from a specialised assembly language (**qasm**) to binary. There is also a full CPU specification, so that other implementations can be written. 

---

## qcpu specification

The **qcpu** has a 16 bit addressable space, where each address contains a 16 bit unsigned integer. Programs are loaded into memory from binary files with each 16 bit number being represented by 2 bytes (low byte and high byte, in that order), and are loaded starting at address 0 - more detail on what these bytes represent will be given later. Any address larger than the length of the binary file will be zeroed.

**qcpu** has 6 registers, typically referred to by the single character names `a b c d x y`, which also store 16 bit unsigned integers.

The `x` and `y` registers are usually considered 'clobberable', meaning their values do not have to be preserved when calling a subroutine. Additionally, `x` is traditionally used as both an argument register and return register for subroutines, with `y` also filling this role if more arguments or return values are needed (if more argument or return values are needed, the other registers can be used, or the stack). If a subroutine would clobber registers `a b c d` then it is common for subroutines to push these values to the stack before executing then popping them back into their respective registers before returning. All of these practices are however not strictly necessary and are only mentioned to allow users to write cleaner, more idiomatic code for the CPU.

There are 4 addressing modes which **qcpu** operands can use:

1. *Immediate* - a numeric constant
2. *Absolute* - value at a given address
3. *Indirect* - value at an address stored in a register
4. *Register* - used to refer to one of the 6 registers.

Attempting to write data to a value with *immediate* addressing mode is undefined behaviour, and therefore implementation-defined. In this implementation, it causes a crash upon trying to write the data.

Additionally, giving a value outside the range of 0-5 (mapping to registers `a b c d x y`) for the *indirect* and *register* addressing modes is undefined behaviour, and therefore implementation-defined. This implementation reacts by crashing, but other implementations are free to react in other ways (for example, by using the modulo operation to map other values into the range 0-5).

**qcpu** has 25 opcodes. These are as follows:

| **value** | **mnemonic** | **effect** |
| ----------- | --------------- | ------------ |
| 0         | `nop`         | does nothing | 
| 1         | `ext a`       | stop exection, returns value `a` |
| 2         | `sys a`       | executes system call `a` (usually accepting argument in register `x`) |
| | **data operations** | |
| 3         | `mov a b` | sets the value in `a` to the value in `b` |
| | **jumps and conditionals** | |
| 4 | `jmp a` | jump to address `a` |
| 5 | `jeq a b c` | jump to address `a` if `b == c` |
| 6 | `jne a b c` | jump to address `a` if `b != c` |
| 7 | `jgt a b c` | jump to address `a` if `b > c` |
| 8 | `jge a b c` | jump to address `a` if `b >= c` |
| 9 | `jlt a b c` | jump to address `a` if `b < c` |
| 10 | `jle a b c` | jump to address `a` if `b <= c` |
| | **subroutines** | |
| 11 | `jsr a` | push the current address to the call stack and jump to address `a` |
| 12 | `ret` | pop an address from the call stack and jump to that address |
| | **arithmetic operations** | |
| 13 | `add a b` | add `b` to the contents of `a` |
| 14 | `sub a b` | subtract `b` from the contents of `a` |
| 15 | `mul a b` | multiply the contents of `a` by `b` |
| 16 | `mod a b` | set the contents of `a` to `a % b` |
| | **bitwise operations** | |
| 17 | `and a b` | set the contents of `a` to the bitwise *and* of `a` with `b` |
| 18 | `orr a b` | set the contents of `a` to the bitwise *or* of `a` with `b` |
| 19 | `not a` | perform a bitwise *not* on the contents of `a` |
| 20 | `xor a b` | set the contents of `a` to the bitwise xor of `a` with `b` |
| 21 | `lsl a b` | perform a logical left shift by `b` bits on the contents of `a` |
| 22 | `lsr a b` | perform a logical right shift by `b` bits on the contents of `a` |
| | **stack operations** | |
| 23 | `psh a` | push value of `a` onto stack |
| 24 | `pop a` | pop top value from stack into `a` |

Addressing mode information for each opcode is stored in the high byte of each instruction, and the opcode itself is stored in the low byte. Addressing mode information for each operand is represented as a 2 bit value, mapping to the addressing modes in the list above. The following diagram shows how the modes are mapped to a byte (remember that in binary files, the low byte containing the opcode is stored first, followed by the high byte containing the addressing modes):

```
aabbccdd oooooooo
| | | |  |
| | | |  -------- 8 bit opcode number
| | | ----------- addressing mode for 4th operand
| | ------------- addressing mode for 3rd operand
| --------------- addressing mode for 2nd operand     
----------------- addressing mode for 1st operand 
```

This allows for opcodes which accept up to 4 different operands. For opcodes with less than 4 operands (which is currently all opcodes), the addressing modes for the unused operands are simply ignored.

The `sys` opcode allows for the CPU to call implementation-specific functions. As an example, the following syscalls are defined in the terminal-based implementation of **qcpu**:

| **syscall** | **effect** |
| --- | --- |
| `sys 6` | writes the character represented by the ASCII character code in register `x` to the output |
| `sys 7` | reads a character from input and moves its ACII character code representation into register `x` |
| `sys 11` | sets the foreground colour of subsequent terminal output to a predefined palette index (0-8) given by register `x` |
| `sys 11` | sets the background colour of subsequent terminal output to a predefined palette index (0-8) given by register `x` |
| `sys 15` | writes the current memory contents to a text file |

This allows for **qcpu** to be used for different purposes, as the syscalls provided by different uses of the CPU can be as complex and varied as desired. A maximum number of 65535 syscalls can be provided. 