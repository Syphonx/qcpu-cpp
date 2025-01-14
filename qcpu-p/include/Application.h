//
//	Application
//

#pragma once

#include "QCPU.h"
#include "Display.h"
#include "Platform.h"
#include "Timer.h"

#include <stdint.h>
#include <string>

class Application
{
public:

	Application(const std::string& filename);
	~Application();

	void ReadInput(const std::string& filename);
	int32_t Run();

	void Bind();
	void Bind_0x06();
	void Bind_0x07();
	void Bind_0x15();
	void Bind_0x16();
	void Bind_0x17();
	void Bind_0x18();
	void Bind_0x19();
	void Bind_0x20();
	void Bind_0x0B();
	void Bind_0x0C();

	void Update();
	void Render();

	bool IsRunning() const;
	QCPU& GetCpu();

private:

	Timer m_BenchTimer;
	Timer m_Timer;
	QCPU m_Cpu;
	Platform m_Platform;
	Display m_Display;

	std::fstream outputfile;
	std::string m_Filename;
	bool m_IsRunning;
	std::string m_Input;
	int32_t m_InputIndex;
};
