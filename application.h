//
// 
//

#pragma once

#include "qcpu.h"
#include "display.h"
#include "platform.h"

#include <string>

const uint16_t SCREEN_WIDTH = 512;
const uint16_t SCREEN_HEIGHT = 512;

struct Timer
{
	Uint64	Last;

	Timer()
	{
		Reset();
	}

	double	ElapsedSeconds()
	{
		return	(double)(((Now() - Last) * 1000 / (double)SDL_GetPerformanceFrequency()) * 0.001);
	}

	double	ElapsedMilliSeconds()
	{
		return	(double)((Now() - Last) * 1000 / (double)SDL_GetPerformanceFrequency());
	}

	Uint64	Now()
	{
		return SDL_GetPerformanceCounter();
	}

	void	Reset()
	{
		Last = Now();
	}
};

class Application
{
public:
	Application(const std::string& filename)
		: m_Cpu()
		, m_Platform(SCREEN_WIDTH, SCREEN_HEIGHT)
		, m_Display(TEXTURE_WIDTH, TEXTURE_HEIGHT, EDisplayMode::Vectron)
		, m_Filename(filename)
		, m_IsRunning(true)
	{
		Bind();
	}

	~Application()
	{
		m_Platform.Shutdown();
	}

	int32_t Run()
	{
		while (IsRunning())
		{
			m_Platform.PumpEvents();
			if (!m_Platform.IsRunning())
			{
				m_IsRunning = false;
			}

			Update();
			
			const float frameMs = (1.0f / 60.0f) * 1000.f;
			if (timer.ElapsedMilliSeconds() > frameMs)
			{
				timer.Reset();
				Render();
			}
		}

		return 0;
	}

	void Bind()
	{
		GetCpu().Load(m_Filename);
		GetCpu().Bind(0x06, [this](const OpArgs& args) { Bind_0x06(); });
		GetCpu().Bind(0x07, [this](const OpArgs& args) { Bind_0x07(); });
		GetCpu().Bind(0x15, [this](const OpArgs& args) { Bind_0x15(); });
		GetCpu().Bind(0x16, [this](const OpArgs& args) { Bind_0x16(); });
		GetCpu().Bind(0x17, [this](const OpArgs& args) { Bind_0x17(); });
		GetCpu().Bind(0x18, [this](const OpArgs& args) { Bind_0x18(); });
		GetCpu().Bind(0x19, [this](const OpArgs& args) { Bind_0x19(); });
		GetCpu().Bind(0x20, [this](const OpArgs& args) { Bind_0x20(); });
		GetCpu().Bind(0x0B, [this](const OpArgs& args) { Bind_0x0B(); });
		GetCpu().Bind(0x0C, [this](const OpArgs& args) { Bind_0x0C(); });
	}

	void Bind_0x06()
	{
		std::cout << static_cast<char>(GetCpu().registers.x);
	}

	void Bind_0x07()
	{
		std::cin >> GetCpu().registers.x;
	}

	void Bind_0x15()
	{
		m_Display.SetDrawMode(EDrawMode::Disabled); 
		m_Display.MoveCursor(GetCpu().registers.x, GetCpu().registers.y); 
		m_Display.SetDrawMode(EDrawMode::Enabled); 
		m_Display.MoveCursor(GetCpu().registers.a, GetCpu().registers.b);
		m_Display.Flush();
	}

	void Bind_0x16()
	{
		m_Display.SetDrawMode(EDrawMode::Disabled); 
		m_Display.MoveCursor(GetCpu().registers.x, GetCpu().registers.y); 
		m_Display.SetDrawMode(EDrawMode::Enabled);
		m_Display.Flush();
	}

	void Bind_0x17()
	{
		m_Display.MoveCursor(GetCpu().registers.x, GetCpu().registers.y); 
		m_Display.Flush();
	}

	void Bind_0x18()
	{
		m_Display.SetColour(EColourMode::Brightness, GetCpu().registers.x);
	}

	void Bind_0x19()
	{
		m_Display.SetColour(EColourMode::Binary, GetCpu().registers.x);
	}

	void Bind_0x20()
	{
		m_Cpu.flags.blok = true;
	}

	void Bind_0x0B()
	{
	}

	void Bind_0x0C()
	{
	}

	bool IsRunning() const
	{
		return m_IsRunning;
	}

	void Update()
	{
		if (m_Cpu.flags.halt == 1)
		{
			// Waiting for resume...
		}
		else
		{
			if (!m_Cpu.flags.blok && m_Cpu.flags.exit == -1)
			{
				m_Cpu.Step();
			}
		}
	}

	void Render()
	{
		m_Cpu.flags.blok = false;
		m_Platform.Update(m_Display, m_Cpu);
		m_Display.Tick();
		m_Platform.Render();
		m_Display.ClearFlush();
	}

	QCPU& GetCpu()
	{
		return m_Cpu;
	}

private:

	Timer							timer;
	QCPU							m_Cpu;
	Platform						m_Platform;
	Display							m_Display;

	std::string						m_Filename;
	bool							m_IsRunning;
};
