//
// 
//

#pragma once

#include <SDL.h>
#include <SDL_stdinc.h>
#include <glad/glad.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui/ext/imgui_memory_editor.h>
#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_opengl3.h>

const uint16_t TEXTURE_WIDTH = 128;
const uint16_t TEXTURE_HEIGHT = 128;

struct Terminal
{
};

class Platform
{
public:
	Platform(uint16_t width, uint16_t height)
		: m_Width(width)
		, m_Height(height)
		, m_IsRunning(true)
		, m_Window(nullptr)
		, m_Renderer(nullptr)
		, m_Texture(nullptr)
	{
		Init();
	}

	void Init()
	{
		SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
			return;
		}

		const char* glsl_version = "#version 130";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		int32_t windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
		m_Window = SDL_CreateWindow(
			"QCPU",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			m_Width,
			m_Height,
			windowFlags
		);

		if (m_Window == nullptr)
		{
			std::cout << "Could not create window: " << SDL_GetError() << std::endl;
			return;
		}

		m_GlContext = SDL_GL_CreateContext(m_Window);
		SDL_GL_MakeCurrent(m_Window, m_GlContext);
		SDL_GL_SetSwapInterval(1);

		if (gladLoadGL() == 0)
		{
			std::cout << "Failed to initialize OpenGL loader!" << std::endl;
			return;
		}

		int32_t renderFlags = SDL_RENDERER_ACCELERATED;
		m_Renderer = SDL_CreateRenderer(m_Window, -1, renderFlags);
		if (!m_Renderer)
		{
			std::cout << "Could not create renderer: " << SDL_GetError() << std::endl;
			return;
		}

		m_Texture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, TEXTURE_WIDTH, TEXTURE_HEIGHT);
		if (!m_Texture)
		{
			std::cout << "Could not create texture from surface: " << SDL_GetError() << std::endl;
			return;
		}

		m_PixelFormat = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// Set the font
		io.Fonts->AddFontDefault();

		ImGui::StyleColorsDark();
		ImGui_ImplSDL2_InitForOpenGL(m_Window, m_GlContext);
		ImGui_ImplOpenGL3_Init(glsl_version);
	}

	void Shutdown()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		SDL_GL_DeleteContext(m_GlContext);
		SDL_DestroyRenderer(m_Renderer);
		SDL_DestroyWindow(m_Window);
		SDL_Quit();
	}

	void Update(const Display& display, QCPU& cpu)
	{
		UpdateDisplay(display);
		UpdateCPU(cpu);
	}

	void Render()
	{
		ImGui::Render();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(114, 144, 154, 255);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
			SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
		}

		SDL_GL_SwapWindow(m_Window);
	}

	void PumpEvents()
	{
		ImGuiIO& io = ImGui::GetIO();
		SDL_Event e;
		int wheel = 0;

		if (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				m_IsRunning = false;
			}
			else if (e.type == SDL_WINDOWEVENT)
			{
				if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					io.DisplaySize.x = static_cast<float>(e.window.data1);
					io.DisplaySize.y = static_cast<float>(e.window.data2);
				}
			}
			else if (e.type == SDL_MOUSEWHEEL)
			{
				wheel = e.wheel.y;
			}
		}

		int mouseX, mouseY;
		const int buttons = SDL_GetMouseState(&mouseX, &mouseY);
		io.DeltaTime = 1.0f / 60.0f;
		io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
		io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
		io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
		io.MouseWheel = static_cast<float>(wheel);
	}

	double GetTime() const
	{
		return SDL_GetTicks();
	}

	bool IsRunning() const
	{
		return m_IsRunning;
	}

private:
	void UpdateDisplay(const Display& display)
	{
		// Get the size of the texture.
		int w, h;
		Uint32 format;
		SDL_QueryTexture(m_Texture, &format, nullptr, &w, &h);

		// Lock the texture, ready to set the pixels
		uint8_t* pixels = nullptr;
		int pitch = 0;
		SDL_LockTexture(m_Texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);

		// Update the texture
		const std::vector<uint8_t>& src = display.GetPixels();
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
		// memcpy(pixels, src.data(), src.size());

		// Finally, unlock the texture
		SDL_UnlockTexture(m_Texture);
	}

	void UpdateCPU(QCPU& cpu)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(m_Window);
		ImGui::NewFrame();

	#if 1
		ImGui::ShowDemoWindow();
	#endif

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::BeginMenu("Open Recent"))
				{
					static std::string files[6] = {
						"programs/bf",
						"programs/bitcount",
						"programs/colortest",
						"programs/pixel",
						"programs/pong",
						"programs/testbench"
					};

					for (size_t i = 0; i < 6; i++)
					{
						if (ImGui::MenuItem(files[i].c_str())) 
						{
							cpu.Load(files[i].c_str());
						}
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if (ImGui::Begin("Display"))
		{
			ImGui::Image(m_Texture, ImVec2(TEXTURE_WIDTH, TEXTURE_HEIGHT));
			ImGui::End();
		}

		m_MemoryEditor.DrawWindow("Memory Editor", cpu.memory, 0xFFFF);

		if (ImGui::Begin("Debugger"))
		{
			if (cpu.flags.halt == 0)
			{
				if (ImGui::Button("Pause"))
				{
					cpu.flags.halt = 1;
				}
			}
			else
			{
				if (ImGui::Button("Continue"))
				{
					cpu.flags.halt = 0;
				}
			}

			if (cpu.flags.halt == 0)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			ImGui::SameLine();

			if (ImGui::Button("Step"))
			{
				cpu.Step();
			}

			if (cpu.flags.halt == 0)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}

			ImGui::End();
		}

		if (ImGui::Begin("CPU"))
		{
			ImGui::Text("PC: %d", cpu.pc);

			ImGui::Text("Registers");
			ImGui::Text("A: %d", cpu.registers.a); ImGui::SameLine();
			ImGui::Text("B: %d", cpu.registers.b); ImGui::SameLine();
			ImGui::Text("C: %d", cpu.registers.c); ImGui::SameLine();
			ImGui::Text("D: %d", cpu.registers.d);
			ImGui::Text("X: %d", cpu.registers.x); ImGui::SameLine();
			ImGui::Text("Y: %d", cpu.registers.y);

			ImGui::Text("Flags");
			ImGui::Text("Halt: %d", cpu.flags.halt);
			ImGui::Text("Stop: %d", cpu.flags.stop);

			ImGui::End();
		}
	}

private:
	int32_t						m_Width;
	int32_t						m_Height;
	bool						m_IsRunning;

	MemoryEditor				m_MemoryEditor;

	SDL_Window*					m_Window;
	SDL_Renderer*				m_Renderer;
	SDL_Texture*				m_Texture;
	SDL_GLContext				m_GlContext;
	SDL_PixelFormat*			m_PixelFormat;
};