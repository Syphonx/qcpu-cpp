//
// qcpu c++ version
//

#include <stdio.h>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <array>
#include <stack>

#include <SDL.h>
#include <SDL_stdinc.h>
#include <iostream>

const uint16_t TEXTURE_WIDTH = 128;
const uint16_t TEXTURE_HEIGHT = 128;
const uint16_t SCREEN_WIDTH = 512;
const uint16_t SCREEN_HEIGHT = 512;
const uint16_t MEMORY_SIZE = 0xFFFF; // 65536

enum EAddressingMode : uint8_t
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
		: halt(-1)
	{
	}

	int16_t halt;
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
	{
		std::fill(memory, memory + MEMORY_SIZE, 0);
	}

public:
	void Load(const std::string& filename)
	{
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
		flags.halt = Read(code);
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
	uint16_t pc;
	uint16_t memory[MEMORY_SIZE];
	Registers registers;
	Flags flags;
	std::stack<uint16_t> callStack;
	std::stack<uint16_t> stack;
	std::vector<OpArgs> zip;
	std::unordered_map<uint16_t, std::function<void(const OpArgs&)>> syscalls;
};

class Application
{
	struct Platform
	{
		Platform(uint16_t width, uint16_t height)
			: m_Width(width)
			, m_Height(height)
			, m_Window(nullptr)
			, m_Renderer(nullptr)
			, m_Texture(nullptr)
		{
		}

		void Init()
		{
			SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
			if (SDL_Init(SDL_INIT_VIDEO) != 0)
			{
				SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
				return;
			}

			int32_t windowFlags = SDL_WINDOW_OPENGL;
			m_Window = SDL_CreateWindow(
				"QCPU",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				m_Width,
				m_Height,
				windowFlags
			);
			if (m_Window == nullptr)
			{
				std::cout << "Could not create window: " << SDL_GetError() << std::endl;
				return;
			}

			//int32_t renderFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
			int32_t renderFlags = SDL_RENDERER_ACCELERATED;
			m_Renderer = SDL_CreateRenderer(m_Window, -1, renderFlags);
			if (!m_Renderer)
			{
				std::cout << "Could not create renderer: " << SDL_GetError() << std::endl;
				return;
			}

			//	SDL_RendererInfo info;
			//	SDL_GetRendererInfo(m_Renderer, &info);
			//	std::cout << "Renderer name: " << info.name << std::endl;
			//	std::cout << "Texture formats: " << std::endl;
			//	for (uint32_t i = 0; i < info.num_texture_formats; i++)
			//	{
			//		std::cout << SDL_GetPixelFormatName(info.texture_formats[i]) << std::endl;
			//	}

			m_Texture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, TEXTURE_WIDTH, TEXTURE_HEIGHT);
			if (!m_Texture)
			{
				std::cout << "Could not create texture from surface: " << SDL_GetError() << std::endl;
				return;
			}

			m_PixelFormat = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
		}

		void Shutdown()
		{
			SDL_DestroyWindow(m_Window);
			SDL_Quit();
		}

		void UpdateTexture(const std::vector<uint8_t>& src)
		{
			// Get the size of the texture.
			int w, h;
			Uint32 format;
			SDL_QueryTexture(m_Texture, &format, nullptr, &w, &h);

			uint8_t* pixels = nullptr;
			int pitch = 0;
			SDL_LockTexture(m_Texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);

			// memcpy(pixels, src.data(), src.size());
			for (int32_t y = 0; y < h; y++)
			{
				int32_t		index = y * pitch;
				uint32_t*	p = (uint32_t*)(pixels + pitch * y); // cast for a pointer increments by 4 bytes.(RGBA)
				for (int32_t x = 0; x < w; x++)
				{
					*p = SDL_MapRGBA(m_PixelFormat,
									 src[index + 0],
									 src[index + 1],
									 src[index + 2],
									 src[index + 3]);
					p++;
					index += (sizeof(uint8_t) * 4);
				}
			}

			SDL_UnlockTexture(m_Texture);
		}

		void Render()
		{
			SDL_SetRenderDrawColor(m_Renderer, 0x00, 0x00, 0x00, 0x00);
			SDL_RenderClear(m_Renderer);
			SDL_RenderCopy(m_Renderer, m_Texture, nullptr, nullptr);
			SDL_RenderPresent(m_Renderer);
		}

		int32_t						m_Width;
		int32_t						m_Height;
		SDL_Window*					m_Window;
		SDL_Renderer*				m_Renderer;
		SDL_Texture*				m_Texture;
		SDL_PixelFormat*			m_PixelFormat;
	};

	enum class EDrawMode : uint8_t
	{
		Disabled,
		Enabled
	};

	enum class EDisplayMode : uint8_t
	{
		Vectron,
		Optron
	};

	enum class EColourMode : uint8_t
	{
		Brightness,
		Binary
	};

	struct Display
	{
		Display(const uint16_t width, const uint16_t height, const EDisplayMode displayMode)
			: m_Width(width)
			, m_Height(height)
			, m_DisplayMode(displayMode)
			, m_Pixels()
			, m_Colour(65535)
		{
			Init();

			m_DisplayMode = EDisplayMode::Vectron;
			m_DrawMode = EDrawMode::Enabled;
			m_ColourMode = EColourMode::Brightness;
			m_CursorX = 0;
			m_CursorY = 0;
		}

		void Init()
		{
			m_Pixels.resize(GetBufferSize());
			std::fill(m_Pixels.begin(), m_Pixels.end(), 0);
		}

		std::vector<uint8_t>& GetPixels()
		{
			return m_Pixels;
		}

		void SetDrawMode(const EDrawMode drawMode)
		{
			m_DrawMode = drawMode;
		}

		void MoveCursor(uint16_t x, uint16_t y)
		{
			switch (m_DrawMode)
			{
				case EDrawMode::Enabled:
				{
					DrawLine(m_CursorX, m_CursorY, x, y);
				}
				break;

				default:
				case EDrawMode::Disabled:
				{
				}
				break;
			}

			m_CursorX = x;
			m_CursorY = y;
		}

		void SetColour(EColourMode mode, uint16_t colour)
		{
			m_ColourMode = mode;
			m_Colour = colour;
		}

	private:

		int32_t GetPixelWidth() const
		{
			return 4; // RGBA
		}

		int32_t GetPixelDepth() const
		{
			return 8 + 8 + 8 + 8; // RGBA
		}

		int32_t GetBufferSize() const
		{
			return m_Height * GetPitch();
		}

		int32_t GetPitch() const
		{
			return m_Width * GetPixelWidth();
		}

		int32_t GetIndex(uint16_t x, uint16_t y) const
		{
			return (m_Width * 4 * y) + x * 4;
		}

		void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
		{
			switch (m_ColourMode)
			{
				default:
				case EColourMode::Brightness:
				{
					uint8_t colour = GetColourBrightness();
					DrawLine(WrapPixel(x0), WrapPixel(y0), WrapPixel(x1), WrapPixel(y1), colour, colour, colour, colour);
				}
				break;

				case EColourMode::Binary:
				{
					if (m_Colour == 0)
					{
						DrawLine(WrapPixel(x0), WrapPixel(y0), WrapPixel(x1), WrapPixel(y1), 255, 255, 255, 255);	// White
					}
					else
					{
						DrawLine(WrapPixel(x0), WrapPixel(y0), WrapPixel(x1), WrapPixel(y1), 255, 0, 0, 255);		// Red
					}
				}
				break;
			}
		}

		void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		{
			bool steep = false;
			if (std::abs(x0 - x1) < std::abs(y0 - y1))
			{
				std::swap(x0, y0);
				std::swap(x1, y1);
				steep = true;
			}
			if (x0 > x1)
			{
				std::swap(x0, x1);
				std::swap(y0, y1);
			}
			int16_t dx = x1 - x0;
			int16_t dy = y1 - y0;
			int16_t derror2 = std::abs(dy) * 2;
			int16_t error2 = 0;
			int16_t y = y0;
			for (int16_t x = x0; x <= x1; x++)
			{
				if (steep)
				{
					DrawPixel(y, x, r, g, b, a);
				}
				else
				{
					DrawPixel(x, y, r, g, b, a);
				}
				error2 += derror2;
				if (error2 > dx)
				{
					y += (y1 > y0 ? 1 : -1);
					error2 -= dx * 2;
				}
			}
		}

		uint8_t GetColourBrightness() const
		{
			return (m_Colour / 65535) * 255;
		}

		uint16_t WrapPixel(uint16_t pixel)
		{
			switch (m_DisplayMode)
			{
				default:
				case EDisplayMode::Optron:
				case EDisplayMode::Vectron:
				{
					return pixel % m_Width;
				}
				break;
			}
		}

		void DrawPixel(uint16_t x, uint16_t y)
		{
			switch (m_ColourMode)
			{
				default:
				case EColourMode::Brightness:
				{
					uint8_t colour = GetColourBrightness();
					DrawPixel(WrapPixel(x), WrapPixel(y), colour, colour, colour, colour);
				}
				break;

				case EColourMode::Binary:
				{
					if (m_Colour == 0)
					{
						DrawPixel(WrapPixel(x), WrapPixel(y), 255, 255, 255, 255);	// White
					}
					else
					{
						DrawPixel(WrapPixel(x), WrapPixel(y), 255, 0, 0, 255);		// Red
					}
				}
				break;
			}
		}

		void DrawPixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		{
			const uint32_t offset = GetIndex(x, y);
			m_Pixels[offset + 0] = b;				// b
			m_Pixels[offset + 1] = g;				// g
			m_Pixels[offset + 2] = r;				// r
			m_Pixels[offset + 3] = a;				// a
		}

		uint16_t					m_Width;
		uint16_t					m_Height;
		std::vector<uint8_t>		m_Pixels;

		EDisplayMode				m_DisplayMode;
		EDrawMode					m_DrawMode;
		EColourMode					m_ColourMode;

		uint16_t					m_Colour;
		uint16_t					m_CursorX;
		uint16_t					m_CursorY;
	};

public:
	Application()
		: m_Cpu()
		, m_Platform(SCREEN_WIDTH, SCREEN_HEIGHT)
		, m_Display(TEXTURE_WIDTH, TEXTURE_HEIGHT, EDisplayMode::Vectron)
		, m_IsRunning(true)
		, m_LastTime()
		, m_FixedTimeAccumulator()
		, m_FixedTimeStep(1 / 60.f)
	{
		Init();
	}

	~Application()
	{
		m_Platform.Shutdown();
	}

	void Init()
	{
		m_Platform.Init();
		m_Display.Init();
		Bind();
	}

	int32_t Run()
	{
		m_LastTime = SDL_GetTicks() / 1000.f;
		while (IsRunning())
		{
			double		time = SDL_GetTicks() / 1000.f;
			double		step = (double)(time - m_LastTime);
			m_LastTime = time;

			SDL_Event e;
			if (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT)
				{
					m_IsRunning = false;
				}
			}

			m_FixedTimeAccumulator += step;
			while (m_FixedTimeAccumulator > m_FixedTimeStep)
			{
				Update();
				m_FixedTimeAccumulator -= m_FixedTimeStep;
			}

			Render();
		}

		return 0;
	}

	void Bind()
	{
		GetCpu().Load("./programs/pong");
		GetCpu().Bind(0x06, [this](const OpArgs& args) { std::cout << static_cast<char>(GetCpu().registers.x); });
		GetCpu().Bind(0x06, [this](const OpArgs& args) { std::cin >> GetCpu().registers.x; });
		GetCpu().Bind(0x15, [this](const OpArgs& args) { m_Display.SetDrawMode(EDrawMode::Disabled); m_Display.MoveCursor(GetCpu().registers.x, GetCpu().registers.y); m_Display.SetDrawMode(EDrawMode::Enabled); m_Display.MoveCursor(GetCpu().registers.a, GetCpu().registers.b); });
		GetCpu().Bind(0x16, [this](const OpArgs& args) { m_Display.SetDrawMode(EDrawMode::Disabled); m_Display.MoveCursor(GetCpu().registers.x, GetCpu().registers.y); m_Display.SetDrawMode(EDrawMode::Enabled); });
		GetCpu().Bind(0x17, [this](const OpArgs& args) { m_Display.MoveCursor(GetCpu().registers.x, GetCpu().registers.y); });
		GetCpu().Bind(0x18, [this](const OpArgs& args) { m_Display.SetColour(EColourMode::Brightness, GetCpu().registers.x); });
		GetCpu().Bind(0x19, [this](const OpArgs& args) { m_Display.SetColour(EColourMode::Binary, GetCpu().registers.x); });
		GetCpu().Bind(0x0B, [this](const OpArgs& args) {});
		GetCpu().Bind(0x0C, [this](const OpArgs& args) {});
	}

	bool IsRunning() const
	{
		return m_IsRunning;
	}

	void Update()
	{
		if (m_Cpu.flags.halt == -1)
		{
			m_Cpu.Step();
		}
	}

	void Render()
	{
		m_Platform.UpdateTexture(m_Display.GetPixels());
		m_Platform.Render();
	}

	QCPU& GetCpu()
	{
		return m_Cpu;
	}

private:

	QCPU			m_Cpu;
	Platform		m_Platform;
	Display			m_Display;

	bool			m_IsRunning;
	double			m_LastTime;
	double			m_FixedTimeAccumulator;
	double			m_FixedTimeStep;
};

int main(int argc, char *argv[])
{
	Application app;
	return app.Run();
}