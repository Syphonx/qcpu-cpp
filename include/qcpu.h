//
//	QCPU
//

#pragma once

#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <stack>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

const uint16_t MEMORY_SIZE = 0xFFFF; // 65536

enum class EAddressingMode : uint8_t
{
	IMMEDIATE = 0b00,
	ABSOLUTE = 0b01,
	INDIRECT = 0b10,
	REGISTER = 0b11,
};

enum class EOpCode : uint8_t
{
	// system operations
	NOP = 0x0,  // does nothing
	EXT = 0x1,  // stop exection, returns value a
	SYS = 0x2,  // executes system call a (usually accepting argument in register x)

	// data operations
	MOV = 0x3,  // sets the value in a to the value in b

	// jumps and conditionals
	JMP = 0x4,  // jump to address a
	JEQ = 0x5,  // jump to address a if b == c
	JNE = 0x6,  // jump to address a if b != c
	JGT = 0x7,  // jump to address a if b > c
	JGE = 0x8,  // jump to address a if b >= c
	JLT = 0x9,  // jump to address a if b < c
	JLE = 0xA,  // jump to address a if b <= c

	// subroutines
	JSR = 0xB,  // push the current address to the call stack and jump to address a
	RET = 0xC,  // pop an address from the call stack and jump to that address

	// arithmetic operations	
	ADD = 0xD,  // add b to the contents of a
	SUB = 0xE,  // subtract b from the contents of a
	MUL = 0xF,  // multiply the contents of a by b
	MDL = 0x10, // set the contents of a to a % b

	// bitwise operations	
	AND = 0x11, // set the contents of a to the bitwise and of a with b
	ORR = 0x12, // set the contents of a to the bitwise or of a with b
	NOT = 0x13, // perform a bitwise not on the contents of a
	XOR = 0x14, // set the contents of a to the bitwise xor of a with b
	LSL = 0x15, // perform a logical left shift by b bits on the contents of a
	LSR = 0x16, // perform a logical right shift by b bits on the contents of a

	// stack operations	
	PSH = 0x17, // push value of a onto stack
	POP = 0x18, // 	pop top value from stack into a
};

enum class EDebugState
{
	Paused,
	Running
};

struct OpArgs
{
	OpArgs(const uint16_t value, const EAddressingMode mode)
		: value(value)
		, mode(mode)
	{
	}

	uint16_t value;
	EAddressingMode mode;
};

struct Flags
{
	Flags()
		: halt(1)
		, exit(-1)
		, blok(false)
	{
	}

	int16_t halt;
	int16_t exit;
	bool	blok;
};

struct Registers
{
	Registers()
		: a(0)
		, b(0)
		, c(0)
		, d(0)
		, x(0)
		, y(0)
	{
	}

	uint16_t a;
	uint16_t b;
	uint16_t c;
	uint16_t d;
	uint16_t x;
	uint16_t y;
};

class QCPU
{
	using SysCallMap = std::unordered_map<uint16_t, std::function<void(const OpArgs&)>>;

public:

	QCPU();

public:

	void Load(const std::string& filename);
	void Reset();

	uint16_t GetArity(const EOpCode opcode) const;
	std::array<EAddressingMode, 4> GetAddressingModes(const uint16_t address) const;

	void ZipArgs(const uint16_t start, const uint16_t end, const std::array<EAddressingMode, 4>& modes);
	void ExecuteOp(const EOpCode opcode);
	void Step();

	void Write(const OpArgs to, const uint16_t val);
	void WriteReg(const uint16_t to, const uint16_t val);

	uint16_t Read(const OpArgs from);
	uint16_t ReadReg(const uint16_t from);

	void Bind(uint16_t value, const std::function<void(const OpArgs&)>& callback);

private:

	void LoadInternal(const std::vector<uint8_t>& data);

private:

	void cpu_nop();
	void cpu_ext(const OpArgs code);
	void cpu_sys(const OpArgs args);
	void cpu_mov(const OpArgs to, const OpArgs from);
	void cpu_jmp(const OpArgs addr);
	void cpu_jeq(const OpArgs addr, const OpArgs b, const OpArgs c);
	void cpu_jne(const OpArgs addr, const OpArgs b, const OpArgs c);
	void cpu_jgt(const OpArgs addr, const OpArgs b, const OpArgs c);
	void cpu_jge(const OpArgs addr, const OpArgs b, const OpArgs c);
	void cpu_jlt(const OpArgs addr, const OpArgs b, const OpArgs c);
	void cpu_jle(const OpArgs addr, const OpArgs b, const OpArgs c);
	void cpu_jsr(const OpArgs addr);
	void cpu_ret();
	void cpu_add(const OpArgs a, const OpArgs b);
	void cpu_sub(const OpArgs a, const OpArgs b);
	void cpu_mul(const OpArgs a, const OpArgs b);
	void cpu_mdl(const OpArgs a, const OpArgs b);
	void cpu_and(const OpArgs a, const OpArgs b);
	void cpu_orr(const OpArgs a, const OpArgs b);
	void cpu_not(const OpArgs a);
	void cpu_xor(const OpArgs a, const OpArgs b);
	void cpu_lsl(const OpArgs a, const OpArgs b);
	void cpu_lsr(const OpArgs a, const OpArgs b);
	void cpu_psh(const OpArgs a);
	void cpu_pop(const OpArgs a);

public:

	uint16_t pc;
	uint16_t cycleCount;
	uint16_t memory[MEMORY_SIZE];
	Registers registers;
	Flags flags;
	std::stack<uint16_t> callStack;
	std::stack<uint16_t> stack;
	std::vector<OpArgs> zip;
	SysCallMap syscalls;
	
	bool debug;
	EDebugState debugState;
};