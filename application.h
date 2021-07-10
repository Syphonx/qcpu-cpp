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
			Render();
		}

		return 0;
	}

	void Bind()
	{
		GetCpu().Load(m_Filename);
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
		if (m_Cpu.flags.halt == 1)
		{
			// Waiting for resume...
		}
		else
		{
			if (m_Cpu.flags.stop == -1)
			{
				m_Cpu.Step();
			}
		}

		m_Platform.Update(m_Display, m_Cpu);
	}

	void Render()
	{
		m_Platform.Render();
	}

	QCPU& GetCpu()
	{
		return m_Cpu;
	}

private:

	QCPU							m_Cpu;
	Platform						m_Platform;
	Display							m_Display;

	std::string						m_Filename;
	bool							m_IsRunning;
};
