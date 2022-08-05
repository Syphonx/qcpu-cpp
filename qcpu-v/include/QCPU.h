//
//	QCPU
//

#pragma once

#include "AddressingMode.h"
#include "Flags.h"
#include "OpArgs.h"
#include "OpCode.h"
#include "Registers.h"

#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <stack>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

enum class EDebugState
{
	Paused,
	Running
};

class QCPU
{
	using SysCallMap = std::unordered_map<uint16_t, std::function<void(const OpArgs&)>>;

public:

	static const uint16_t MEMORY_SIZE = 0xFFFF; // 65536

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