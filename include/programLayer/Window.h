#pragma once

#include <iostream>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <fstream>
#include "TextEditor.h"
#include "FileLogger.h"
#include <Translator.h>
#include <SubWindows.h>

struct ItemAction {
	const char* name;
	void (*action)(); // Pointer to a function for the item's action
};

enum ProgrammingLanguages {
	ASM_4BIT = 0,
};

class Window {
public:
	~Window();
	void CreateMainWindow(std::string directFileToOpen = "");
	void Update();
	void Render();
	void RenderMainMenuBar();

	void ButtonSave();										//Saves text(code) from current active text editor
	void ButtonOpen();										//Creates new project with main editor text from given document
	void ButtonCreateNew();									//Creates new project

	void LoadTempalte(std::string templateName);
	void OpenFile();
	bool SaveFile();

	GLFWwindow* GetNativeWindow() const { return m_window; }
private:
	static void windowFocusCallback(GLFWwindow* window, int focused);
	void LoadTextures();
	void UiToolbar();
	void CopyToClipboard(const std::string& text);
private:
	GLFWwindow* m_window;
	TextEditor m_mainEditor;
	Interpreter* m_interpreter = nullptr;
	SubWindows* m_subWindows = nullptr;
	TextEditor::Coordinates cpos;
	Translator translator;

	ProgrammingLanguages currentProgramminLanguage = ProgrammingLanguages::ASM_4BIT;

	//Textures
	unsigned int m_textureIDPlayButton;
	unsigned int m_textureIDStopButton;
	unsigned int m_textureIDToolbarBg;
	unsigned int m_textureIDClipboard;
	unsigned int m_textureIDLampOff;
    unsigned int m_textureIDLampOn;
	unsigned int m_textureIDStepButton;

	bool m_showMain = true;
};