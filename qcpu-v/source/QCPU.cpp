//
//	QCPU
//

#include "QCPU.h"

QCPU::QCPU()
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
	memset(&memory[0], 0, MEMORY_SIZE);
}

void QCPU::Load(const std::string& filename)
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

void QCPU::Reset()
{
	memset(&memory[0], 0, MEMORY_SIZE);
	pc = 0;
	registers = Registers();
	flags = Flags();
	callStack = std::stack<uint16_t>();
	stack = std::stack<uint16_t>();
	zip.clear();
}

uint16_t QCPU::GetArity(const EOpCode opcode) const
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

std::array<EAddressingMode, 4> QCPU::GetAddressingModes(const uint16_t address) const
{
	return {
		static_cast<EAddressingMode>((address & 0b11000000) >> 6),
		static_cast<EAddressingMode>((address & 0b00110000) >> 4),
		static_cast<EAddressingMode>((address & 0b00001100) >> 2),
		static_cast<EAddressingMode>((address & 0b00000011))
	};
}

void QCPU::ZipArgs(const uint16_t start, const uint16_t end, const std::array<EAddressingMode, 4>& modes)
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

void QCPU::ExecuteOp(const EOpCode opcode)
{
	static bool opcode_debug = false;
	if (opcode_debug)
	{
		printf("Exectuing opcode: %s\n", EnumToString(opcode));
	}

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

void QCPU::Step()
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
	cycleCount++;

	ExecuteOp(opcode);
}

void QCPU::Write(const OpArgs to, const uint16_t val)
{
	switch (to.mode)
	{
		default:
		case EAddressingMode::Imm:
		{
			std::cout << "cannot write to immediate value: " << to.value << std::endl;
		}
		break;

		case EAddressingMode::Abs:
		{
			memory[to.value] = val;
		}
		break;

		case EAddressingMode::Ind:
		{
			memory[Read({ to.value, EAddressingMode::Reg })] = val;
		}
		break;

		case EAddressingMode::Reg:
		{
			WriteReg(to.value, val);
		}
		break;
	}
}

void QCPU::WriteReg(const uint16_t to, const uint16_t val)
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

uint16_t QCPU::Read(const OpArgs from)
{
	switch (from.mode)
	{
		default:
		case EAddressingMode::Imm:
		{
			return from.value;
		}
		break;

		case EAddressingMode::Abs:
		{
			return memory[from.value];
		}
		break;

		case EAddressingMode::Ind:
		{
			return memory[Read({ from.value, EAddressingMode::Reg })];
		}
		break;

		case EAddressingMode::Reg:
		{
			return ReadReg(from.value);
		}
		break;
	}
}

uint16_t QCPU::ReadReg(const uint16_t from)
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

void QCPU::Bind(uint16_t value, const std::function<void(const OpArgs&)>& callback)
{
	syscalls.emplace(value, callback);
}

void QCPU::LoadInternal(const std::vector<uint8_t>& data)
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
