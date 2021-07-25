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
#include <imgui/ext/texteditor/imgui_texteditor.h>

const uint16_t TEXTURE_WIDTH = 512;
const uint16_t TEXTURE_HEIGHT = 512;

// Shader sources
const GLchar* vertexSource = R"glsl(
	#version 330 core
	layout (location = 0) in vec2 aPos;
	layout (location = 1) in vec3 aColor;
	layout (location = 2) in vec2 aTexCoord;
	out vec3 outColor;
	out vec2 TexCoord;
	void main()
	{
		gl_Position = vec4(aPos, 1.0, 1.0);
		outColor = aColor;
		TexCoord = vec2(aTexCoord.x, aTexCoord.y);
	}
)glsl";

const GLchar* fragmentSource = R"glsl(
	#version 330 core
	out vec4 FragColor;
	in vec3 outColor;
	in vec2 TexCoord;
	uniform sampler2D inTexture;
	void main()
	{
		FragColor = texture(inTexture, TexCoord);
	}
)glsl";

struct Texture
{
	Texture()
	{
	}

	void Create()
	{
		glGenTextures(1, &inTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, inTexture);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		std::vector<GLubyte> pixels;
		int pixelIndex = 0;
		pixels.clear();
		pixels.resize(TEXTURE_WIDTH * TEXTURE_HEIGHT * 4);

		for (size_t i = 0; i < TEXTURE_WIDTH * TEXTURE_HEIGHT * 4; i++)
		{
			pixels[i] = rand() % 255;
		}

		glTexImage2D(
			GL_TEXTURE_2D,			// target
			0,						// mip
			GL_RGBA, 				// format
			TEXTURE_WIDTH, 			// width
			TEXTURE_HEIGHT, 		// heigh
			0, 						// border
			GL_BGRA, 				// format
			GL_UNSIGNED_INT_8_8_8_8_REV, 		// type
			pixels.data()			// pixels
		);

		glBindTexture(GL_TEXTURE_2D, 0); // unbind
	}

	GLuint					inTexture;
};

struct Shader
{
	void ValidateShader(GLuint shader, const std::string& type)
	{
		GLint					success;
		std::vector<GLchar>		infoLog;
		GLint					maxLength = 256;

		if (type != "Program")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
				infoLog.resize(maxLength);

				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
				printf("Failed to compile shader %s (%s)", type.c_str(), &infoLog[0]);
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
				infoLog.resize(maxLength);

				glGetProgramInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
				printf("Failed to link shader %s (%s)", type.c_str(), &infoLog[0]);
			}
		}
	}

	void Create()
	{
		// Create and compile the vertex shader
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		glCompileShader(vertexShader);
		ValidateShader(vertexShader, "Vertex");

		// Create and compile the fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		glCompileShader(fragmentShader);
		ValidateShader(fragmentShader, "Fragment");

		// Link the vertex and fragment shader into a shader program
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		ValidateShader(shaderProgram, "Program");

		glUseProgram(shaderProgram);
	}

	void Use()
	{
		glUseProgram(shaderProgram);
	}

	GLuint shaderProgram;
};

struct Quad
{
	Quad()
	{
	}

	void Create(const Shader& shader)
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);

		GLfloat vertices[] = {
			-1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
			 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
			 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
			-1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
		};

		GLuint elements[] = {
			0, 1, 3, // first triangle
			1, 2, 3  // second triangle
		};

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

		// Specify the layout of the vertex data
		GLint posAttrib = glGetAttribLocation(shader.shaderProgram, "aPos");
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

		GLint colAttrib = glGetAttribLocation(shader.shaderProgram, "aColor");
		glEnableVertexAttribArray(colAttrib);
		glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

		GLint texAttrib = glGetAttribLocation(shader.shaderProgram, "aTexCoord");
		glEnableVertexAttribArray(texAttrib);
		glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
	}

	GLuint vao;
	GLuint vbo;
	GLuint ebo;
};

class Platform
{
public:
	Platform(uint16_t width, uint16_t height)
		: m_Width(width)
		, m_Height(height)
		, m_IsRunning(true)
		, m_Window(nullptr)
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
		oldW = m_Width;
		oldH = m_Height;

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
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// Set the font
		io.Fonts->AddFontDefault();
		ImGui::StyleColorsDark();
		ImGui_ImplSDL2_InitForOpenGL(m_Window, m_GlContext);
		ImGui_ImplOpenGL3_Init(glsl_version);

		// Create shaders/textures
		quadShader.Create();
		quadMesh.Create(quadShader);
		quadTexture.Create();

		quadShader.Use();
		glUniform1i(glGetUniformLocation(quadShader.shaderProgram, "inTexture"), 0);

		std::vector<std::string> lines;
		lines.clear();
		std::ifstream file(R"(D:\Portfolio\qcpu\qcpu-cpp\asm\pong.asm)");
		std::string s;
		while (getline(file, s))
		{
			lines.push_back(s);
		}

		m_TextEditor.SetTextLines(lines);
	}

	void Shutdown()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		SDL_GL_DeleteContext(m_GlContext);
		SDL_DestroyWindow(m_Window);
		SDL_Quit();
	}

	void Update(Display& display, QCPU& cpu)
	{
		if (display.ShouldFlush())
		{
			glBindTexture(GL_TEXTURE_2D, quadTexture.inTexture);
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

	void Render()
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		RenderDisplay();
		RenderUI();
		SDL_GL_SwapWindow(m_Window);
	}

	void PumpEvents()
	{
		ImGuiIO& io = ImGui::GetIO();
		SDL_Event e;
		int wheel = 0;

		while (SDL_PollEvent(&e))
		{
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

					if (e.type == SDL_MOUSEWHEEL)
					{
						wheel = e.wheel.y;
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

							if (newW > oldW || newH > oldH)
							{
								newW = std::max(newW, newH);
								newH = std::max(newW, newH);
							}
							else
							{
								newW = std::min(newW, newH);
								newH = std::min(newW, newH);
							}

							oldW = newW;
							oldH = newH;
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
	void UpdateUI(Display& display, QCPU& cpu)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(m_Window);
		ImGui::NewFrame();

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;
			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

	#if 0
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
							display.Init();
							cpu.Load(files[i].c_str());
						}
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

	#if 0
		if (ImGui::Begin("Display"))
		{
			ImGui::Image((ImTextureID)quadTexture.inTexture, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
			ImGui::End();
		}
	#endif

	#if 1
		m_MemoryEditor.DrawWindow("Memory Editor", cpu.memory, 0xFFFF);
	#endif

	#if 1
		m_TextEditor.Render("Text viewer");
	#endif

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
			ImGui::Text("Stop: %d", cpu.flags.exit);

			ImGui::End();
		}
	}

	void RenderDisplay()
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, quadTexture.inTexture);

		quadShader.Use();

		glBindVertexArray(quadMesh.vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	void RenderUI()
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

private:
	int32_t						m_Width;
	int32_t						m_Height;
	bool						m_IsRunning;

	MemoryEditor				m_MemoryEditor;
	ImGui::Ext::TextEditor		m_TextEditor;

	SDL_Window*					m_Window;
	SDL_GLContext				m_GlContext;

	Shader						quadShader;
	Quad						quadMesh;
	Texture						quadTexture;
	int							oldW, oldH;
};