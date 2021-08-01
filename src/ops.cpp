//
//	QCPU
//

#include "qcpu.h"

void QCPU::cpu_nop()
{

}

void QCPU::cpu_ext(const OpArgs code)
{
	flags.exit = Read(code);
}

void QCPU::cpu_sys(const OpArgs args)
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

void QCPU::cpu_mov(const OpArgs to, const OpArgs from)
{
	Write(to, Read(from));
}

void QCPU::cpu_jmp(const OpArgs addr)
{
	pc = Read(addr);
}

void QCPU::cpu_jeq(const OpArgs addr, const OpArgs b, const OpArgs c)
{
	if (Read(b) == Read(c))
	{
		cpu_jmp(addr);
	}
}

void QCPU::cpu_jne(const OpArgs addr, const OpArgs b, const OpArgs c)
{
	if (Read(b) != Read(c))
	{
		cpu_jmp(addr);
	}
}

void QCPU::cpu_jgt(const OpArgs addr, const OpArgs b, const OpArgs c)
{
	if (Read(b) > Read(c))
	{
		cpu_jmp(addr);
	}
}

void QCPU::cpu_jge(const OpArgs addr, const OpArgs b, const OpArgs c)
{
	if (Read(b) >= Read(c))
	{
		cpu_jmp(addr);
	}
}

void QCPU::cpu_jlt(const OpArgs addr, const OpArgs b, const OpArgs c)
{
	if (Read(b) < Read(c))
	{
		cpu_jmp(addr);
	}
}

void QCPU::cpu_jle(const OpArgs addr, const OpArgs b, const OpArgs c)
{
	if (Read(b) <= Read(c))
	{
		cpu_jmp(addr);
	}
}

void QCPU::cpu_jsr(const OpArgs addr)
{
	callStack.push(pc);
	cpu_jmp(addr);
}

void QCPU::cpu_ret()
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

void QCPU::cpu_add(const OpArgs a, const OpArgs b)
{
	uint16_t read_a = Read(a);
	uint16_t read_b = Read(b);
	Write(a, read_a + read_b);
}

void QCPU::cpu_sub(const OpArgs a, const OpArgs b)
{
	uint16_t read_a = Read(a);
	uint16_t read_b = Read(b);
	Write(a, read_a - read_b);
}

void QCPU::cpu_mul(const OpArgs a, const OpArgs b)
{
	uint16_t read_a = Read(a);
	uint16_t read_b = Read(b);
	Write(a, read_a * read_b);
}

void QCPU::cpu_mdl(const OpArgs a, const OpArgs b)
{
	uint16_t read_a = Read(a);
	uint16_t read_b = Read(b);
	Write(a, read_a % read_b);
}

void QCPU::cpu_and(const OpArgs a, const OpArgs b)
{
	uint16_t read_a = Read(a);
	uint16_t read_b = Read(b);
	Write(a, read_a & read_b);
}

void QCPU::cpu_orr(const OpArgs a, const OpArgs b)
{
	uint16_t read_a = Read(a);
	uint16_t read_b = Read(b);
	Write(a, read_a | read_b);
}

void QCPU::cpu_not(const OpArgs a)
{
	uint16_t read_a = Read(a);
	Write(a, ~read_a);
}

void QCPU::cpu_xor(const OpArgs a, const OpArgs b)
{
	uint16_t read_a = Read(a);
	uint16_t read_b = Read(b);
	Write(a, read_a ^ read_b);
}

void QCPU::cpu_lsl(const OpArgs a, const OpArgs b)
{
	uint16_t read_a = Read(a);
	uint16_t read_b = Read(b);
	Write(a, read_a << read_b);
}

void QCPU::cpu_lsr(const OpArgs a, const OpArgs b)
{
	uint16_t read_a = Read(a);
	uint16_t read_b = Read(b);
	Write(a, read_a >> read_b);
}

void QCPU::cpu_psh(const OpArgs a)
{
	uint16_t read_a = Read(a);
	stack.push(read_a);
}

void QCPU::cpu_pop(const OpArgs a)
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