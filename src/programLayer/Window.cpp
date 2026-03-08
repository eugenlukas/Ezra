#define STB_IMAGE_IMPLEMENTATION
#define IMGUI_DEFINE_MATH_OPERATORS

#include "Window.h"
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include "FileDialogs.h"
#include <Windows.h>
#include "imguitablabel.h"
#include "stb_image/stb_image.h"
#include "imgui_canvas.h"
#include "TextureLoader.h"
#include "ThemeManager.h"
#include "StringHelper.h"

Window::~Window()
{
	delete m_subWindows;
	delete m_interpreter;
}

void Window::CreateMainWindow(std::string directFileToOpen)
{
	LOG("Started");
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	m_window = glfwCreateWindow(1920, 1080, "Ezra", NULL, NULL);

	// Error check if the window fails to create
	if (m_window == NULL)
	{
		LOG("Failed to create GLFW window");
		glfwTerminate();
		return;
	}

	// Change window icon
	int iconWidth, iconHeight, iconChannels;
	std::string iconPath = "resources/Icon.png";
	unsigned char* iconPixels = stbi_load(iconPath.c_str(), &iconWidth, &iconHeight, &iconChannels, 4);
	GLFWimage images[1];
	images[0].width = iconWidth;
	images[0].height = iconHeight;
	images[0].pixels = iconPixels;
	glfwSetWindowIcon(m_window, 1, images);

	// Maximize the window
	glfwMaximizeWindow(m_window);

	// Introduce the window into the current context
	glfwMakeContextCurrent(m_window);

	// Set the window focus callback
	glfwSetWindowFocusCallback(m_window, windowFocusCallback);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, 800, 800);

	// Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	ThemeManager::SetApplicationDarkStyle();

	// Load Textures
	LoadTextures();
	LOG("Loaded Textures");

	// Initialize Main Text Editor
	auto lang = TextEditor::LanguageDefinition::ASM_4BIT();
	m_mainEditor.addOnCharTypedCallback([this](ImWchar aChar) { /*someFunc()*/ });
	m_mainEditor.SetReadOnly(false);
	m_mainEditor.LoadTempalte("new_file");
	m_mainEditor.SetReadOnly(true);
	m_mainEditor.SetReadOnly(true);
	m_mainEditor.SetLanguageDefinition(lang);

	// Initialize Interpreter
	m_interpreter = new Interpreter();

	// Initialize Sub Windows
	m_subWindows = new SubWindows(m_interpreter, &m_mainEditor, &m_textureIDLampOff, &m_textureIDLampOn);

	//Load old saved ui
	ImGui::LoadIniSettingsFromDisk("resources/Layouts/default.ini");

	//Open given file directly if there is one or create default
	if (!(directFileToOpen == ""))
	{
		m_mainEditor.SetFilepath(directFileToOpen);
		std::filesystem::path fsPath(directFileToOpen);
		std::string parentPath = fsPath.parent_path().string();
		OpenFile();
	}
	/*else
		LoadTempalte("new_file");*/

	//Load all languages and set the default language
	translator.LoadLanguages();
	translator.LoadLanguage("en_en");

	// Main while loop
	while (!glfwWindowShouldClose(m_window))
	{
		Update();
		Render();
	}

	// Deletes all ImGUI instances
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	// Delete window before ending the program
	glfwDestroyWindow(m_window);
	// Terminate GLFW before ending the program
	glfwTerminate();
}

void Window::windowFocusCallback(GLFWwindow* window, int focused)
{
	if (focused)
	{
		LOG("Window gained focus");
	}
	else
	{
		LOG("Window lost focus");
	}
}

void Window::LoadTextures()
{
	TextureLoader::LoadTexture("resources/PlayButton.png", &m_textureIDPlayButton);
	TextureLoader::LoadTexture("resources/StopButton.png", &m_textureIDStopButton);
	TextureLoader::LoadTexture("resources/ToolbarBg.png", &m_textureIDToolbarBg);
	TextureLoader::LoadTexture("resources/Clipboard.png", &m_textureIDClipboard);
	TextureLoader::LoadTexture("resources/Lamp_off.png", &m_textureIDLampOff);
	TextureLoader::LoadTexture("resources/Lamp_on.png", &m_textureIDLampOn);
	TextureLoader::LoadTexture("resources/Step.png", &m_textureIDStepButton);
}

void Window::Update()
{
	cpos = m_mainEditor.GetCursorPosition();

	//Hotkey/Shortcut Listener
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_S))
		ButtonSave();
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_O))
		ButtonOpen();
	if (ImGui::IsKeyPressed(ImGuiKey_F9, false))
	{
		m_interpreter->Run();
	}
}

void Window::Render()
{
	// Specify the color of the background
	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Begin new ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	RenderMainMenuBar();

	// Docking configuration
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	 
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar;

	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		window_flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		window_flags |= ImGuiWindowFlags_NoBackground; // Transparent background
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpace", nullptr, window_flags);

	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	dockspace_flags |= ImGuiDockNodeFlags_PassthruCentralNode;

	ImGuiID dockspace_id = ImGui::GetID("DockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	// Docking setup (ensuring persistent layout)
	if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
	{
		ImGui::DockBuilderRemoveNode(dockspace_id); // Reset dock
		ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_None);
		ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

		ImGuiID dock_top = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.75f, nullptr, &dockspace_id);
		ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dock_top);

		ImGui::DockBuilderDockWindow("Code Editor", dock_top);
		ImGui::DockBuilderDockWindow("Console", dock_bottom);
		ImGui::DockBuilderFinish(dockspace_id);
	}

	ImGui::End();

	// Code Editor Window
	if (m_showMain)
	{
		ImGui::Begin("Code Editor");

		if (ImGui::BeginTabBar("EditorTabs"))
		{
			if (ImGui::BeginTabItem(m_mainEditor.GetFilepath().empty() ? "Main file" : m_mainEditor.GetFilepath().c_str()))
			{
				ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s",
					cpos.mLine + 1, m_mainEditor.GetConvertedColumn() + 1, m_mainEditor.GetTotalLines(),
					m_mainEditor.IsOverwrite() ? "Ovr" : "Ins",
					m_mainEditor.CanUndo() ? "*" : " ",
					m_mainEditor.GetLanguageDefinition().mName.c_str(),
					m_mainEditor.GetFilepath().c_str());

				m_mainEditor.Render("Editor");
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
		ImGui::End();
	}

	// Binary Window
	m_subWindows->Render();

	UiToolbar();

	// Renders the ImGUI elements
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Swap the back buffer with the front buffer
	glfwSwapBuffers(m_window);
	// Take care of all GLFW events
	glfwPollEvents();
}

void Window::UiToolbar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0.5f));

	ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	float size = ImGui::GetWindowHeight() - 4.0f;
	float buttonSize = size - 10;
	float buttonSpacing = 25.0f; // Space between the two buttons
	float backgroundWidth = (buttonSize * 5) + buttonSpacing; // Make background match buttons
	float backgroundHeight = buttonSize + 6.0f; // Add slight padding for aesthetics

	// ---- Center Background Independently ----
	float bgPosX = (ImGui::GetContentRegionMax().x * 0.5f) - (backgroundWidth * 0.5f) - 4;
	float bgPosY = (ImGui::GetContentRegionAvail().y * 0.5f) - (backgroundHeight * 0.5f);
	ImGui::SetCursorPos(ImVec2(bgPosX, bgPosY + 2.5f));
	ImGui::Image(m_textureIDToolbarBg, ImVec2(backgroundWidth, backgroundHeight));

	// ---- Center Buttons Independently ----
	float buttonPosX = (ImGui::GetContentRegionMax().x * 0.5f) - ((buttonSize * 5 + buttonSpacing) * 0.5f);
	float buttonPosY = bgPosY + ((backgroundHeight - buttonSize) * 0.5f); // Align vertically in background

	ImGui::SetCursorPos(ImVec2(buttonPosX, buttonPosY));

	// Check if compiled program is running
	bool isRunning = m_interpreter->IsRunning();

	// Grey out start button when the compiled program is already running or not file is opend
	if (isRunning)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}

	//Render PlayButton
	if (ImGui::ImageButton("PlayButtonTex", m_textureIDPlayButton, ImVec2(buttonSize, buttonSize)))
	{
		m_interpreter->Run();
	}

	// Grey out start button when the compiled program is already running or not file is opend
	if (isRunning)
	{
		// Pop greyed out styling
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	ImGui::SameLine();

	// Grey out stop button when the compiled program is not running
	if (!isRunning)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}

	//Render StopButton
	if (ImGui::ImageButton("StopButtonTex", m_textureIDStopButton, ImVec2(buttonSize, buttonSize)))
	{
		m_interpreter->Stop();
	}

	// Grey out stop button when the compiled program is not running
	if (!isRunning)
	{
		// Pop greyed out styling
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	ImGui::SameLine();

	if (ImGui::ImageButton("StepButtonTex", m_textureIDStepButton, ImVec2(buttonSize + 10, buttonSize)))
	{
		m_interpreter->Step();
	}

	ImGui::Separator();
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(3);
	ImGui::End();
}

void Window::CopyToClipboard(const std::string& text)
{
	// Open the clipboard
	if (!OpenClipboard(nullptr)) return;

	// Empty the clipboard
	EmptyClipboard();

	// Allocate global memory for the text (plus null terminator)
	HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
	if (!hGlob)
	{
		CloseClipboard();
		return;
	}

	// Copy the string into the global memory
	memcpy(GlobalLock(hGlob), text.c_str(), text.size() + 1);
	GlobalUnlock(hGlob);

	// Set clipboard data
	SetClipboardData(CF_TEXT, hGlob);

	// Close the clipboard
	CloseClipboard();
}

void Window::RenderMainMenuBar()
{
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0, 0, 0, 0));

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu(translator.Get("File")))
		{
			if (ImGui::MenuItem(translator.Get("NewFile")))
			{
				delete m_interpreter;
				m_interpreter = new Interpreter();

				delete m_subWindows;
				m_subWindows = new SubWindows(m_interpreter, &m_mainEditor, &m_textureIDLampOff, &m_textureIDLampOn);

				m_mainEditor.SetReadOnly(false);
				m_mainEditor.SelectAll();
				m_mainEditor.Delete();
				m_mainEditor.LoadTempalte("new_file");
				m_mainEditor.SetReadOnly(true);

				m_mainEditor.SetFilepath("");
			}
			if (ImGui::MenuItem(translator.Get("OpenFile")))
			{
				ButtonOpen();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu(translator.Get("Edit")))
		{
			if (m_mainEditor.GetFilepath() == "")
				ImGui::BeginDisabled();
			if (m_mainEditor.GetFilepath() == "")
				ImGui::EndDisabled();
			if (ImGui::MenuItem(translator.Get("Save"), "Ctrl+S"))
			{
				ButtonSave();
			}
			if (ImGui::MenuItem(translator.Get("SaveAs")))
			{
				std::string savePath = FileDialogs::SaveFile("", *this);
				if (savePath != "")
				{
					m_mainEditor.SetFilepath(savePath);
					SaveFile();
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu(translator.Get("RunStepReset")))
		{
			if (ImGui::MenuItem(m_interpreter->IsRunning() ? translator.Get("Stop") : translator.Get("Run")))
				m_interpreter->IsRunning() ? m_interpreter->Stop() : m_interpreter->Run();
			ImGui::SliderFloat(translator.Get("Speed"), m_interpreter->GetRunningSpeed(), 0.05f, 5.0f);

			if (ImGui::MenuItem(translator.Get("Step")))
				m_interpreter->Step();
			if (ImGui::MenuItem(translator.Get("Reset")))
				m_interpreter->Reset();
			
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu(translator.Get("View")))
		{
			if (ImGui::BeginMenu(translator.Get("Language")))
			{
				for (std::string langCode : translator.GetAllLangCodes())
				{
					if (ImGui::MenuItem(langCode.c_str()))
					{
						translator.LoadLanguage(langCode);
					}
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	ImGui::PopStyleColor(2);
}

void Window::ButtonSave()
{
	if (m_mainEditor.GetFilepath() != "")
	{
		if (SaveFile())
			m_mainEditor.MarkLinesSaved();
	}
	else
	{
		std::string savePath = FileDialogs::SaveFile("", *this);
		if (savePath != "")
		{
			m_mainEditor.SetFilepath(savePath);
			if (SaveFile())
			{
				m_mainEditor.MarkLinesSaved();
			}
		}
	}
}

void Window::ButtonOpen()
{
	std::string path = FileDialogs::OpenFile("", *this);

	if (path == "")
		return;

	m_mainEditor.SetFilepath(path);
	OpenFile();
}

void Window::ButtonCreateNew()
{
	m_mainEditor.SetFilepath("");

	m_mainEditor.SelectAll();
	m_mainEditor.Delete();

	LoadTempalte("new_file");
}

void Window::LoadTempalte(std::string templateName)
{
	std::string templatePath = "resources/Templates/" + (templateName + ".txt");

	std::fstream newfile;
	newfile.open(templatePath, std::ios::in); //open a file to perform read operation using file object
	if (newfile.is_open())
	{ //checking whether the file is open
		std::string tp;
		while (getline(newfile, tp))
		{ //read data from file object and put it into string.
			m_mainEditor.InsertText(tp + "\n", true);
		}
		newfile.close(); //close the file object.
	}
}

void Window::OpenFile()
{
	m_mainEditor.SetReadOnly(false);

	m_mainEditor.SelectAll();
	m_mainEditor.Delete();

	std::fstream newfile;
	std::string filepath = m_mainEditor.GetFilepath();
	newfile.open(filepath, std::ios::in); //open a file to perform read operation using file object
	if (newfile.is_open())
	{ //checking whether the file is open
		std::string tp;
		uint8_t index = 0;
		while (getline(newfile, tp))
		{ //read data from file object and put it into string.
			m_mainEditor.InsertText(tp, false);
			if (!newfile.eof())
				m_mainEditor.InsertText("\n", false);

			LOG("Write to flash with line: '" + tp + "'");
			m_interpreter->WriteFlashRowFromSave(tp, index);

			index++;
		}
		newfile.close(); //close the file object.
	}

	// Auto suggest programming language ASM when asm4 file is opened
	if (filepath.substr(filepath.find_last_of(".") + 1) == "asm4")
		m_mainEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::ASM_4BIT());

	m_mainEditor.SetReadOnly(true);
}

bool Window::SaveFile()
{
	// Open the file in output mode, truncating it to overwrite existing content
	std::fstream file;
	file.open(m_mainEditor.GetFilepath(), std::ios::out | std::ios::trunc); // open file to write

	if (file.is_open())
	{
		std::string editorText = m_mainEditor.GetText();  // Get all text from the editor

		// Remove newline(s)
		while (!editorText.empty() &&
			(editorText.back() == '\n' || editorText.back() == '\r'))
		{
			editorText.pop_back();
		}

		file << editorText;  // Write the text to the file

		file.close();  // Close the file when done
		return true;
	}
	else
	{
		// Handle file opening failure
		LOG("Failed to open the file for saving.");
		return false;
	}
}
