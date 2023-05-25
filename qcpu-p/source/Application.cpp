//
// Application
//

#include "Application.h"
#include "Constants.h"
#include "OS/Filesystem.h"

#include <fstream>

Application::Application(const std::string& filename)
	: m_Cpu()
	, m_Platform(SCREEN_WIDTH, SCREEN_HEIGHT)
	, m_Display(TEXTURE_WIDTH, TEXTURE_HEIGHT, EDisplayMode::Vectron)
	, m_Filename(filename)
	, m_IsRunning(true)
	, outputfile(std::fstream("log.txt", std::ios::out))
	, m_InputIndex(0)
{
	Bind();
}

Application::~Application()
{
#if WITH_DISPLAY
	m_Platform.Shutdown();
#endif
}

void Application::ReadInput(const std::string& filename)
{
	FileReader reader;
	if (reader.Open(filename, std::ifstream::in))
	{
		m_Input.resize(reader.GetLength());
		reader.Read(reader.GetLength(), m_Input);
	}
}

int32_t Application::Run()
{
	m_Timer.Reset();
	m_BenchTimer.Reset();
	while (IsRunning())
	{
		m_Platform.PumpEvents();
		if (!m_Platform.IsRunning())
		{
			m_IsRunning = false;
		}

		Update();

		const float frameMs = (1.0f / 60.0f) * 1000.f;
		if (m_Timer.ElapsedMilliSeconds() > frameMs)
		{
			m_Timer.Reset();
			Render();
		}
	}

	return 0;
}

void Application::Bind()
{
	m_Cpu.Load(m_Filename);
	m_Cpu.Bind(0x06, [this](const OpArgs& args) { Bind_0x06(); });
	m_Cpu.Bind(0x07, [this](const OpArgs& args) { Bind_0x07(); });
	m_Cpu.Bind(0x15, [this](const OpArgs& args) { Bind_0x15(); });
	m_Cpu.Bind(0x16, [this](const OpArgs& args) { Bind_0x16(); });
	m_Cpu.Bind(0x17, [this](const OpArgs& args) { Bind_0x17(); });
	m_Cpu.Bind(0x18, [this](const OpArgs& args) { Bind_0x18(); });
	m_Cpu.Bind(0x19, [this](const OpArgs& args) { Bind_0x19(); });
	m_Cpu.Bind(0x20, [this](const OpArgs& args) { Bind_0x20(); });
	m_Cpu.Bind(0x0B, [this](const OpArgs& args) { Bind_0x0B(); });
	m_Cpu.Bind(0x0C, [this](const OpArgs& args) { Bind_0x0C(); });
}

void Application::Bind_0x06()
{
	const char ch = static_cast<char>(m_Cpu.registers.x);
	outputfile.write(&ch, sizeof(char));
}

void Application::Bind_0x07()
{
	if (m_InputIndex > m_Input.length())
	{
		m_Cpu.flags.exit = 0;
	}
	else
	{
		m_Cpu.registers.x = m_Input[m_InputIndex++];
	}
}

void Application::Bind_0x15()
{
#if WITH_DISPLAY
	m_Display.SetDrawMode(EDrawMode::Disabled);
	m_Display.MoveCursor(m_Cpu.registers.x, m_Cpu.registers.y);
	m_Display.SetDrawMode(EDrawMode::Enabled);
	m_Display.MoveCursor(m_Cpu.registers.a, m_Cpu.registers.b);
	m_Display.Flush();
#endif
}

void Application::Bind_0x16()
{
#if WITH_DISPLAY
	m_Display.SetDrawMode(EDrawMode::Disabled);
	m_Display.MoveCursor(m_Cpu.registers.x, m_Cpu.registers.y);
	m_Display.SetDrawMode(EDrawMode::Enabled);
	m_Display.Flush();
#endif
}

void Application::Bind_0x17()
{
#if WITH_DISPLAY
	m_Display.MoveCursor(m_Cpu.registers.x, m_Cpu.registers.y);
	m_Display.Flush();
#endif
}

void Application::Bind_0x18()
{
#if WITH_DISPLAY
	m_Display.SetColour(EColourMode::Brightness, m_Cpu.registers.x);
#endif
}

void Application::Bind_0x19()
{
#if WITH_DISPLAY
	m_Display.SetColour(EColourMode::Binary, m_Cpu.registers.x);
#endif
}

void Application::Bind_0x20()
{
	m_Cpu.flags.blok = true;
}

void Application::Bind_0x0B()
{

}

void Application::Bind_0x0C()
{

}

void Application::Update()
{
	static bool benchprint = false;
	if (m_Cpu.flags.exit != -1)
	{
		if (benchprint)
		{
			return;
		}

		const double elapsed = m_BenchTimer.ElapsedMilliSeconds();

		outputfile.close(); 
		std::ifstream f("log.txt");
		std::cout << f.rdbuf();

		printf("\n");
		printf("> exited with code %d \n", m_Cpu.flags.exit);
		printf("> cycle count: %d \n", m_Cpu.cycleCount);
		printf("> execution time: %.3f ms\n", elapsed);
		printf("> ms/cycle: %.3f ms\n", (elapsed / m_Cpu.cycleCount));
		benchprint = true;

		m_IsRunning = false;
		return;
	}

	if (m_Cpu.flags.halt == 1)
	{
		// Waiting for resume...
	}
	else
	{
		if (!m_Cpu.flags.blok)
		{
			m_Cpu.Step();
		}
	}
}

void Application::Render()
{
#if WITH_DISPLAY
	m_Cpu.flags.blok = false;
	m_Platform.Update(m_Display, m_Cpu);
	m_Display.Tick();
	m_Platform.Render();
	m_Display.ClearFlush();
#endif
}

bool Application::IsRunning() const
{
	return m_IsRunning;
}
