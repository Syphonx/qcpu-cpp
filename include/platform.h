//
//	Platform
//

#pragma once

#include "display.h"
#include "qcpu.h"

#include "gl/shader.h"
#include "gl/quad.h"
#include "gl/texture.h"

#include <SDL.h>
#include <SDL_stdinc.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui/ext/imgui_memory_editor.h>
#include <imgui/ext/texteditor/imgui_texteditor.h>

class Platform
{
public:
									Platform(uint16_t width, uint16_t height);

	void							Init();
	void							Shutdown();
	void							Update(Display& display, QCPU& cpu);
	void							Render();
	void							PumpEvents();

	double							GetTime() const;
	bool							IsRunning() const;

private:
	void							UpdateUI(Display& display, QCPU& cpu);
	void							RenderDisplay();
	void							RenderUI();

private:
	int32_t							m_Width;
	int32_t							m_Height;
	bool							m_IsRunning;

	MemoryEditor					m_MemoryEditor;
	ImGui::Ext::TextEditor			m_TextEditor;

	SDL_Window*						m_Window;
	SDL_GLContext					m_GlContext;

	Shader							m_QuadShader;
	Quad							m_QuadMesh;
	Texture							m_QuadTexture;

	int32_t							m_OldWidth;
	int32_t							m_OldHeight;
};