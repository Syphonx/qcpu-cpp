//
//	QCPU
//

#pragma once

#include <stdint.h>

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
	POP = 0x18  // pop top value from stack into a
};

static const char* EnumToString(const EOpCode InOpcode)
{
	switch (InOpcode)
	{
		case EOpCode::NOP: return "NOP";
		case EOpCode::EXT: return "EXT";
		case EOpCode::SYS: return "SYS";
		case EOpCode::MOV: return "MOV";
		case EOpCode::JMP: return "JMP";
		case EOpCode::JEQ: return "JEQ";
		case EOpCode::JNE: return "JNE";
		case EOpCode::JGT: return "JGT";
		case EOpCode::JGE: return "JGE";
		case EOpCode::JLT: return "JLT";
		case EOpCode::JLE: return "JLE";
		case EOpCode::JSR: return "JSR";
		case EOpCode::RET: return "RET";
		case EOpCode::ADD: return "ADD";
		case EOpCode::SUB: return "SUB";
		case EOpCode::MUL: return "MUL";
		case EOpCode::MDL: return "MDL";
		case EOpCode::AND: return "AND";
		case EOpCode::ORR: return "ORR";
		case EOpCode::NOT: return "NOT";
		case EOpCode::XOR: return "XOR";
		case EOpCode::LSL: return "LSL";
		case EOpCode::LSR: return "LSR";
		case EOpCode::PSH: return "PSH";
		case EOpCode::POP: return "POP";
	}

	return "";
}