//
// 
//

#pragma once

#define ASSERTF_DEF_ONCE
#include "assertf.h"

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
		: halt(0)
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
public:
	QCPU()
		: memory()
		, pc(0)
		, registers()
		, flags()
		, callStack()
		, stack()
		, syscalls()
		, debug(false)
		, debugState(EDebugState::Running)
	{
		std::fill(memory, memory + MEMORY_SIZE, 0);
	}

public:
	void Load(const std::string& filename)
	{
		Reset();

		std::ifstream file(filename, std::ios::binary);
		file.unsetf(std::ios::skipws);

		std::streampos fileSize;
		file.seekg(0, std::ios::end);
		fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		if (fileSize <= 0)
		{
			return;
		}

		std::vector<uint8_t> buffer;
		buffer.reserve(fileSize);
		buffer.insert(buffer.begin(),
					  std::istream_iterator<uint8_t>(file),
					  std::istream_iterator<uint8_t>());

		LoadInternal(buffer);
	}

	void Reset()
	{
		std::fill(memory, memory + MEMORY_SIZE, 0);
		pc = 0;
		registers = Registers();
		flags = Flags();
		callStack = std::stack<uint16_t>();
		stack = std::stack<uint16_t>();
		zip.clear();
	}

	uint16_t GetArity(const EOpCode opcode) const
	{
		switch (opcode)
		{
			default:
			case EOpCode::NOP:
			case EOpCode::RET:
			{
				return 0;
			}
			break;

			case EOpCode::SYS:
			case EOpCode::JMP:
			case EOpCode::JSR:
			case EOpCode::NOT:
			case EOpCode::PSH:
			case EOpCode::POP:
			{
				return 1;
			}
			break;

			case EOpCode::EXT:
			case EOpCode::MOV:
			case EOpCode::ADD:
			case EOpCode::SUB:
			case EOpCode::MUL:
			case EOpCode::MDL:
			case EOpCode::AND:
			case EOpCode::ORR:
			case EOpCode::XOR:
			case EOpCode::LSL:
			case EOpCode::LSR:
			{
				return 2;
			}
			break;

			case EOpCode::JEQ:
			case EOpCode::JNE:
			case EOpCode::JGT:
			case EOpCode::JGE:
			case EOpCode::JLT:
			case EOpCode::JLE:
			{
				return 3;
			}
			break;
		}
	}

	std::array<EAddressingMode, 4> GetAddressingModes(const uint16_t address) const
	{
		return {
			static_cast<EAddressingMode>((address & 0b11000000) >> 6),
			static_cast<EAddressingMode>((address & 0b00110000) >> 4),
			static_cast<EAddressingMode>((address & 0b00001100) >> 2),
			static_cast<EAddressingMode>((address & 0b00000011))
		};
	}

	void ZipArgs(const uint16_t start, const uint16_t end, const std::array<EAddressingMode, 4>& modes)
	{
		zip.clear();
		for (size_t i = 0; i < end - start; i++)
		{
			switch (i)
			{
				case 0: {} break;
				case 1:
				case 2:
				case 3:
				case 4: { zip.emplace_back(memory[start + i - 1], modes[i - 1]); } break;
				default:
				{
					std::cout << "invalid number of arguments" << std::endl;
				}
				break;
			}
		}
	}

	void ExecuteOp(const EOpCode opcode)
	{
		switch (opcode)
		{
			case EOpCode::NOP: cpu_nop(); break;
			case EOpCode::EXT: cpu_ext(zip[0]); break;
			case EOpCode::SYS: cpu_sys(zip[0]); break;
			case EOpCode::MOV: cpu_mov(zip[0], zip[1]); break;
			case EOpCode::JMP: cpu_jmp(zip[0]); break;
			case EOpCode::JEQ: cpu_jeq(zip[0], zip[1], zip[2]); break;
			case EOpCode::JNE: cpu_jne(zip[0], zip[1], zip[2]); break;
			case EOpCode::JGT: cpu_jgt(zip[0], zip[1], zip[2]); break;
			case EOpCode::JGE: cpu_jge(zip[0], zip[1], zip[2]); break;
			case EOpCode::JLT: cpu_jlt(zip[0], zip[1], zip[2]); break;
			case EOpCode::JLE: cpu_jle(zip[0], zip[1], zip[2]); break;
			case EOpCode::JSR: cpu_jsr(zip[0]); break;
			case EOpCode::RET: cpu_ret(); break;
			case EOpCode::ADD: cpu_add(zip[0], zip[1]); break;
			case EOpCode::SUB: cpu_sub(zip[0], zip[1]); break;
			case EOpCode::MUL: cpu_mul(zip[0], zip[1]); break;
			case EOpCode::MDL: cpu_mdl(zip[0], zip[1]); break;
			case EOpCode::AND: cpu_and(zip[0], zip[1]); break;
			case EOpCode::ORR: cpu_orr(zip[0], zip[1]); break;
			case EOpCode::NOT: cpu_not(zip[0]); break;
			case EOpCode::XOR: cpu_xor(zip[0], zip[1]); break;
			case EOpCode::LSL: cpu_lsl(zip[0], zip[1]); break;
			case EOpCode::LSR: cpu_lsr(zip[0], zip[1]); break;
			case EOpCode::PSH: cpu_psh(zip[0]); break;
			case EOpCode::POP: cpu_pop(zip[0]); break;
		}
	}

	void Step()
	{
		uint16_t	current = memory[pc];
		uint16_t	op = current & 0x00FF;
		uint16_t	addr = (current & 0xFF00) >> 8;
		EOpCode		opcode = static_cast<EOpCode>(op);

		uint16_t	arity = GetArity(opcode);
		std::array<EAddressingMode, 4> addressing_modes = GetAddressingModes(addr);

		pc += 1;
		ZipArgs(pc, pc + arity + 1, addressing_modes);
		pc += arity;

		ExecuteOp(opcode);
	}

	void Write(const OpArgs to, const uint16_t val)
	{
		switch (to.mode)
		{
			default:
			case EAddressingMode::IMMEDIATE:
			{
				std::cout << "cannot write to immediate value: " << to.value << std::endl;
			}
			break;

			case EAddressingMode::ABSOLUTE:
			{
				memory[to.value] = val;
			}
			break;

			case EAddressingMode::INDIRECT:
			{
				memory[Read({ to.value, EAddressingMode::REGISTER })] = val;
			}
			break;

			case EAddressingMode::REGISTER:
			{
				WriteReg(to.value, val);
			}
			break;
		}
	}

	void WriteReg(const uint16_t to, const uint16_t val)
	{
		switch (to)
		{
			default:
			{
				std::cout << "Unknown register: " << to << std::endl;
			}
			break;

			case 0: { registers.a = val; } break;
			case 1: { registers.b = val; } break;
			case 2: { registers.c = val; } break;
			case 3: { registers.d = val; } break;
			case 4: { registers.x = val; } break;
			case 5: { registers.y = val; } break;
		}
	}

	uint16_t Read(const OpArgs from)
	{
		switch (from.mode)
		{
			default:
			case EAddressingMode::IMMEDIATE:
			{
				return from.value;
			}
			break;

			case EAddressingMode::ABSOLUTE:
			{
				return memory[from.value];
			}
			break;

			case EAddressingMode::INDIRECT:
			{
				return memory[Read({ from.value, EAddressingMode::REGISTER })];
			}
			break;

			case EAddressingMode::REGISTER:
			{
				return ReadReg(from.value);
			}
			break;
		}
	}

	uint16_t ReadReg(const uint16_t from)
	{
		switch (from)
		{
			default:
			{
				std::cout << "Unknown register: %d" << from << std::endl;
				return 0;
			}
			break;

			case 0: { return registers.a; } break;
			case 1: { return registers.b; } break;
			case 2: { return registers.c; } break;
			case 3: { return registers.d; } break;
			case 4: { return registers.x; } break;
			case 5: { return registers.y; } break;
		}
	}

	void Bind(uint16_t value, const std::function<void(const OpArgs&)>& callback)
	{
		syscalls.emplace(value, callback);
	}

private:
	void LoadInternal(const std::vector<uint8_t>& data)
	{
		if (data.size() % 2 != 0)
		{
			std::cout << "Data must be multiple of 2!" << std::endl;
		}

		for (size_t i = 0; i < data.size() / 2; i++)
		{
			uint8_t byte1 = data[i * 2];
			uint8_t byte2 = data[i * 2 + 1];
			memory[i] = ((static_cast<uint16_t>(byte2) << 8) + static_cast<uint16_t>(byte1));
		}
	}

private:
	void cpu_nop()
	{
	}

	void cpu_ext(const OpArgs code)
	{
		flags.exit = Read(code);
	}

	void cpu_sys(const OpArgs args)
	{
		auto iter = syscalls.find(Read(args));
		if (iter != syscalls.end())
		{
			syscalls[Read(args)](args);
		}
		else
		{
			std::cout << "Failed to find syscall: 0x" << std::hex << Read(args) << std::endl;
		}
	}

	void cpu_mov(const OpArgs to, const OpArgs from)
	{
		Write(to, Read(from));
	}

	void cpu_jmp(const OpArgs addr)
	{
		pc = Read(addr);
	}

	void cpu_jeq(const OpArgs addr, const OpArgs b, const OpArgs c)
	{
		if (Read(b) == Read(c))
		{
			cpu_jmp(addr);
		}
	}

	void cpu_jne(const OpArgs addr, const OpArgs b, const OpArgs c)
	{
		if (Read(b) != Read(c))
		{
			cpu_jmp(addr);
		}
	}

	void cpu_jgt(const OpArgs addr, const OpArgs b, const OpArgs c)
	{
		if (Read(b) > Read(c))
		{
			cpu_jmp(addr);
		}
	}

	void cpu_jge(const OpArgs addr, const OpArgs b, const OpArgs c)
	{
		if (Read(b) >= Read(c))
		{
			cpu_jmp(addr);
		}
	}

	void cpu_jlt(const OpArgs addr, const OpArgs b, const OpArgs c)
	{
		if (Read(b) < Read(c))
		{
			cpu_jmp(addr);
		}
	}

	void cpu_jle(const OpArgs addr, const OpArgs b, const OpArgs c)
	{
		if (Read(b) <= Read(c))
		{
			cpu_jmp(addr);
		}
	}

	void cpu_jsr(const OpArgs addr)
	{
		callStack.push(pc);
		cpu_jmp(addr);
	}

	void cpu_ret()
	{
		if (callStack.empty())
		{
			std::cout << "Attempted to pop empty stack!" << std::endl;
		}
		else
		{
			pc = callStack.top();
			callStack.pop();
		}
	}

	void cpu_add(const OpArgs a, const OpArgs b)
	{
		uint16_t read_a = Read(a);
		uint16_t read_b = Read(b);
		Write(a, read_a + read_b);
	}

	void cpu_sub(const OpArgs a, const OpArgs b)
	{
		uint16_t read_a = Read(a);
		uint16_t read_b = Read(b);
		Write(a, read_a - read_b);
	}

	void cpu_mul(const OpArgs a, const OpArgs b)
	{
		uint16_t read_a = Read(a);
		uint16_t read_b = Read(b);
		Write(a, read_a * read_b);
	}

	void cpu_mdl(const OpArgs a, const OpArgs b)
	{
		uint16_t read_a = Read(a);
		uint16_t read_b = Read(b);
		Write(a, read_a % read_b);
	}

	void cpu_and(const OpArgs a, const OpArgs b)
	{
		uint16_t read_a = Read(a);
		uint16_t read_b = Read(b);
		Write(a, read_a & read_b);
	}

	void cpu_orr(const OpArgs a, const OpArgs b)
	{
		uint16_t read_a = Read(a);
		uint16_t read_b = Read(b);
		Write(a, read_a | read_b);
	}

	void cpu_not(const OpArgs a)
	{
		uint16_t read_a = Read(a);
		Write(a, ~read_a);
	}

	void cpu_xor(const OpArgs a, const OpArgs b)
	{
		uint16_t read_a = Read(a);
		uint16_t read_b = Read(b);
		Write(a, read_a ^ read_b);
	}

	void cpu_lsl(const OpArgs a, const OpArgs b)
	{
		uint16_t read_a = Read(a);
		uint16_t read_b = Read(b);
		Write(a, read_a << read_b);
	}

	void cpu_lsr(const OpArgs a, const OpArgs b)
	{
		uint16_t read_a = Read(a);
		uint16_t read_b = Read(b);
		Write(a, read_a >> read_b);
	}

	void cpu_psh(const OpArgs a)
	{
		uint16_t read_a = Read(a);
		stack.push(read_a);
	}

	void cpu_pop(const OpArgs a)
	{
		if (stack.empty())
		{
			std::cout << "Attempted to pop empty stack!" << std::endl;
		}
		else
		{
			uint16_t value = stack.top();
			stack.pop();
			Write(a, value);
		}
	}

public:
	uint16_t						pc;
	uint16_t						memory[MEMORY_SIZE];
	Registers						registers;
	Flags							flags;
	std::stack<uint16_t>			callStack;
	std::stack<uint16_t>			stack;
	std::vector<OpArgs>				zip;
	std::unordered_map<uint16_t, std::function<void(const OpArgs&)>> syscalls;
	
	bool							debug;
	EDebugState						debugState;
};