//
//	Platform
//

#pragma once

#include "DebugInfo.h"
#include "Display.h"
#include "OpenGL/Quad.h"
#include "OpenGL/Shader.h"
#include "OpenGL/Texture.h"
#include "OS/Filesystem.h"
#include "QCPU.h"

#include <SDL.h>
#include <SDL_stdinc.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui/ext/imgui_memory_editor.h>
#include <imgui/ext/texteditor/imgui_texteditor.h>

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

class Debugger
{
public:

	Debugger()
	{
	}

	bool Load(const std::string& program)
	{
		FileReader reader;
		std::string filename = program + ".debug";
		
		if (reader.Open(filename))
		{
			cereal::JSONInputArchive archive(reader.GetStream());
			archive(info);
		
			return true;
		}
		
		return false;
	}

	int32_t GetLine(const QCPU& cpu)
	{
		for (const TokenData& token : info.tokens)
		{
			if (token.address == cpu.pc)
			{
				return token.line;
			}
		}

		return -1;
	}

private:

	DebugInfo info;
};

class Platform
{
public:
	
	Platform(uint16_t width, uint16_t height);

	void Init();
	void Shutdown();
	void Update(Display& display, QCPU& cpu);
	void Render();
	void PumpEvents();

	double GetTime() const;
	bool IsRunning() const;

private:

	void LoadProgram(Display& display, QCPU& cpu, const std::string& program);

	void UpdateUI(Display& display, QCPU& cpu);
	void RenderDisplay();
	void RenderUI();

private:

	int32_t m_Width;
	int32_t m_Height;
	bool m_IsRunning;

	MemoryEditor m_MemoryEditor;
	TextEditor m_TextEditor;
	Debugger m_Debugger;

	SDL_Window* m_Window;
	SDL_GLContext m_GlContext;

	Shader m_QuadShader;
	Quad m_QuadMesh;
	Texture m_QuadTexture;

	int32_t m_OldWidth;
	int32_t m_OldHeight;
};