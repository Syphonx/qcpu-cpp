//
//	Platform
//

#include "Platform.h"
#include "Constants.h"
#include "OS/Filesystem.h"

#include <glad/glad.h>
#include <filesystem>
#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include "assembler.h"

Platform::Platform(uint16_t width, uint16_t height)
	: m_Width(width)
	, m_Height(height)
	, m_IsRunning(true)
	, m_Window(nullptr)
{
#if WITH_DISPLAY
	Init();
#endif
}

void Platform::Init()
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
	m_OldWidth = m_Width;
	m_OldHeight = m_Height;

	if (m_Window == nullptr)
	{
		std::cout << "Could not create window: " << SDL_GetError() << std::endl;
		return;
	}

	m_GlContext = SDL_GL_CreateContext(m_Window);
	SDL_GL_MakeCurrent(m_Window, m_GlContext);
	SDL_GL_SetSwapInterval(0); // v-sync

	if (gladLoadGL() == 0)
	{
		std::cout << "Failed to initialize OpenGL loader!" << std::endl;
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	io.Fonts->AddFontDefault();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(m_Window, m_GlContext);
	ImGui_ImplOpenGL3_Init(glsl_version);

	m_QuadShader.Create(R"(assets\shaders\quad.vs)", R"(assets\shaders\quad.fs)");
	m_QuadMesh.Create(m_QuadShader);
	m_QuadTexture.Create();
	m_QuadShader.Use();
	m_QuadShader.SetInt("inTexture", 0);

	TextEditor::LanguageDefinition lang = TextEditor::LanguageDefinition::QASM();
	for (const Opcode& op : Assembler::ops)
	{
		lang.mKeywords.insert(op.name);
	}

	for (const RegisterData& reg : Assembler::registers)
	{
		TextEditor::Identifier id;
		id.mDeclaration = "Built-in register";
		lang.mIdentifiers.insert(std::make_pair(std::string(reg.name), id));
	}

	static const char* const s_Directives[] = {
		".text", ".org", ".ds"
	};

	for (auto& directive : s_Directives)
	{
		lang.mKeywords.insert(directive);
	}

	m_TextEditor.SetLanguageDefinition(lang);
	m_TextEditor.SetReadOnly(false);
}

void Platform::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	SDL_GL_DeleteContext(m_GlContext);
	SDL_DestroyWindow(m_Window);
	SDL_Quit();
}

void Platform::Update(Display& display, QCPU& cpu)
{
	if (display.ShouldFlush())
	{
		glBindTexture(GL_TEXTURE_2D, m_QuadTexture.m_Id);
		glTexSubImage2D(
			GL_TEXTURE_2D,					// target
			0,								// mip level
			0, 								// x offet
			0, 								// y offset
			TEXTURE_WIDTH, 					// width
			TEXTURE_HEIGHT, 				// heigh
			GL_BGRA, 						// format
			GL_UNSIGNED_INT_8_8_8_8_REV, 	// type
			display.GetPixels().data()		// pixels
		);
	}
	UpdateUI(display, cpu);
}

void Platform::Render()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	RenderDisplay();
	RenderUI();
	SDL_GL_SwapWindow(m_Window);
}

void Platform::PumpEvents()
{
#if WITH_DISPLAY
	ImGuiIO& io = ImGui::GetIO();
	SDL_Event e;
	int wheel = 0;

	while (SDL_PollEvent(&e))
	{
		ImGui_ImplSDL2_ProcessEvent(&e);

		switch (e.type)
		{
			case SDL_KEYDOWN:
			{
				switch (e.key.keysym.sym)
				{
					case SDLK_ESCAPE:
					{
						m_IsRunning = false;
					}
					break;
				}
			}
			break;

			case SDL_WINDOWEVENT:
			{
				switch (e.window.event)
				{
					case SDL_WINDOWEVENT_CLOSE:
					{
						m_IsRunning = false;
					}
					break;

					case SDL_WINDOWEVENT_SIZE_CHANGED:
					{
						float w = static_cast<float>(e.window.data1);
						float h = static_cast<float>(e.window.data2);
						io.DisplaySize.x = w;
						io.DisplaySize.y = h;
					}
					break;

					case SDL_WINDOWEVENT_RESIZED:
					{
						int newW = e.window.data1;
						int newH = e.window.data2;

						if (newW > m_OldWidth || newH > m_OldHeight)
						{
							newW = std::max(newW, newH);
							newH = std::max(newW, newH);
						}
						else
						{
							newW = std::min(newW, newH);
							newH = std::min(newW, newH);
						}

						m_OldWidth = newW;
						m_OldHeight = newH;
						SDL_SetWindowSize(m_Window, newW, newH);
					}
					break;
				}
			}
			break;
		}
	}

	int mouseX, mouseY;
	const int buttons = SDL_GetMouseState(&mouseX, &mouseY);
	io.DeltaTime = 1.0f / 60.0f;
	io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
	io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
	io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
#endif
}

double Platform::GetTime() const
{
	return SDL_GetTicks();
}

bool Platform::IsRunning() const
{
	return m_IsRunning;
}

void Platform::LoadProgram(Display& display, QCPU& cpu, const std::string& program)
{
	display.Init();
	cpu.Load(program.c_str());
}

void Platform::UpdateUI(Display& display, QCPU& cpu)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(m_Window);
	ImGui::NewFrame();

	static bool s_ShowDemoWindow = false;
	static bool s_ShowFpsWindow = true;
	static bool s_ShowDisplayWindow = false;
	static bool s_ShowMemoryWindow = false;
	static bool s_ShowTextWindow = true;
	static bool s_ShowDebugWindow = true;
	static bool s_ShowCPUWindow = true;

	static bool s_DebuggerAttached = false;

	if (s_ShowDemoWindow)
	{
		ImGui::ShowDemoWindow(&s_ShowDemoWindow);
	}

	if (s_ShowFpsWindow)
	{
		static float f = 0.0f;
		static int counter = 0;
		if (ImGui::Begin("Hello, world!", &s_ShowFpsWindow))
		{
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}
		ImGui::End();
	}

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("Run"))
			{
				std::vector<std::string> programs;
				for (const auto& entry : std::filesystem::directory_iterator("programs"))
				{
					const std::filesystem::path& path = entry.path();
					programs.push_back(path.string());
				}

				for (auto& program : programs)
				{
					if (ImGui::MenuItem(program.c_str()))
					{
						LoadProgram(display, cpu, program);
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Windows"))
		{
			ImGui::MenuItem("Show Demo Window", nullptr, &s_ShowDemoWindow);
			ImGui::MenuItem("Show Fps Window", nullptr, &s_ShowFpsWindow);
			ImGui::MenuItem("Show Display Window", nullptr, &s_ShowDisplayWindow);
			ImGui::MenuItem("Show Memory Window", nullptr, &s_ShowMemoryWindow);
			ImGui::MenuItem("Show Text Window", nullptr, &s_ShowTextWindow);
			ImGui::MenuItem("Show Debug Window", nullptr, &s_ShowDebugWindow);
			ImGui::MenuItem("Show CPU Window", nullptr, &s_ShowCPUWindow);

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (s_ShowMemoryWindow)
	{
		m_MemoryEditor.DrawWindow("Memory Editor", cpu.memory, 0xFFFF, &s_ShowMemoryWindow);
	}

	if (s_ShowTextWindow)
	{
		auto cpos = m_TextEditor.GetCursorPosition();
		if (ImGui::Begin("Text Editor", &s_ShowTextWindow, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar))
		{
			static std::string s_CurrentProgramName;
			static std::string s_CurrentProgramPath;

			ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Save", nullptr, false, !s_CurrentProgramPath.empty()))
					{
						FileWriter writer;
						if (writer.Open(s_CurrentProgramPath))
						{
							writer.Write(m_TextEditor.GetText());
						}
					}

					if (ImGui::BeginMenu("Open"))
					{
						std::vector<std::string> programNames;
						std::vector<std::string> programPaths;
						for (const auto& entry : std::filesystem::directory_iterator("asm"))
						{
							const std::filesystem::path& path = entry.path();
							programNames.push_back(path.filename().replace_extension("").string());
							programPaths.push_back(path.string());
						}

						for (int32_t i = 0; i < programPaths.size(); i++)
						{
							if (ImGui::MenuItem(programNames[i].c_str()))
							{
								FileReader reader;
								std::vector<std::string> lines;
								if (reader.Open(programPaths[i]))
								{
									reader.ReadLines(lines);
								}
								s_CurrentProgramPath = programPaths[i];
								s_CurrentProgramName = programNames[i];

								s_DebuggerAttached = false;
								m_TextEditor.SetTextLines(lines);
							}
						}

						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Edit"))
				{
					bool ro = m_TextEditor.IsReadOnly();
					if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
						m_TextEditor.SetReadOnly(ro);
					ImGui::Separator();

					if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && m_TextEditor.CanUndo()))
						m_TextEditor.Undo();
					if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && m_TextEditor.CanRedo()))
						m_TextEditor.Redo();

					ImGui::Separator();

					if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, m_TextEditor.HasSelection()))
						m_TextEditor.Copy();
					if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && m_TextEditor.HasSelection()))
						m_TextEditor.Cut();
					if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && m_TextEditor.HasSelection()))
						m_TextEditor.Delete();
					if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
						m_TextEditor.Paste();

					ImGui::Separator();

					if (ImGui::MenuItem("Select all", nullptr, nullptr))
						m_TextEditor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(m_TextEditor.GetTotalLines(), 0));

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Assemble", !s_CurrentProgramName.empty()))
				{
					bool build = false;
					bool run = false;

					if (ImGui::MenuItem("Build"))
					{
						build = true;
					}

					if (ImGui::MenuItem("Build & Run"))
					{
						build = true;
						run = true;
					}

					if (build)
					{
						Assembler avengers(s_CurrentProgramPath);
						const std::vector<uint8_t>& text = avengers.Assemble();

						std::string newFile = "programs/" + s_CurrentProgramName;
						std::ofstream file(newFile, std::ios::out | std::ios::binary);
						if (!text.empty())
						{
							file.write(reinterpret_cast<const char*>(text.data()), text.size());
						}

						if (run)
						{
							LoadProgram(display, cpu, newFile);
						}

						build = false;
						run = false;
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Debug", !s_CurrentProgramName.empty()))
				{
					if (ImGui::MenuItem("Attach"))
					{
						if (m_Debugger.Load("programs/" + s_CurrentProgramName))
						{
							s_DebuggerAttached = true;
						}
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, m_TextEditor.GetTotalLines(),
						m_TextEditor.IsOverwrite() ? "Ovr" : "Ins",
						m_TextEditor.CanUndo() ? "*" : " ",
						m_TextEditor.GetLanguageDefinition().mName.c_str(), s_CurrentProgramName.c_str());

			m_TextEditor.Render("Text viewer");
		}

		ImGui::End();
	}

	if (s_ShowDebugWindow)
	{
		if (ImGui::Begin("Debugger", &s_ShowDebugWindow))
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
				
				if (s_DebuggerAttached)
				{
					int32_t line = m_Debugger.GetLine(cpu);
					TextEditor::Coordinates coord(line-1, 0);
					TextEditor::Breakpoints bpts;
					bpts.insert(line);
					m_TextEditor.SetBreakpoints(bpts);
					m_TextEditor.SetCursorPosition(coord);
				}
			}

			if (cpu.flags.halt == 0)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();
	}

	if (s_DebuggerAttached)
	{
		int32_t line = m_Debugger.GetLine(cpu);
		TextEditor::Coordinates coord(line - 1, 0);
		TextEditor::Breakpoints bpts;
		bpts.insert(line);
		m_TextEditor.SetBreakpoints(bpts);
		m_TextEditor.SetCursorPosition(coord);
	}

	if (s_ShowCPUWindow)
	{
		if (ImGui::Begin("CPU", &s_ShowCPUWindow))
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
			ImGui::Text("Stop: %d", cpu.flags.exit);
		}
		ImGui::End();
	}
}

void Platform::RenderDisplay()
{
	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_QuadTexture.m_Id);

	m_QuadShader.Use();

	glBindVertexArray(m_QuadMesh.vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Platform::RenderUI()
{
	ImGui::Render();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
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
}

