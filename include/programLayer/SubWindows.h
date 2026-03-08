#pragma once
#include <TextureLoader.h>
#include <TextEditor.h>
#include <Interpreter.h>

class SubWindows
{
public:
    SubWindows(Interpreter* interpreter, TextEditor* editor, unsigned int* textureIDLampOff, unsigned int* textureIDLampOn)
    {
        this->m_interpreter = interpreter;
        this->m_TextEditor = editor;
        this->m_textureIDLampOff = textureIDLampOff;
        this->m_textureIDLampOn = textureIDLampOn;
    }

    void Render();
    void RenderOutput();
private:
    void RenderFlash();
    void RenderALU();
    void RenderCounter();
    void RenderRam();

    void RenderFlashModal(uint8_t rowIndex);
    void WriteChnagesToFlash(int SelectedItem, int rowIndex, int rowValue);

    bool BitButton(const char* id, bool& bit, bool isFlashRow, uint8_t flashRowIndex = 0);

private:
    Interpreter* m_interpreter;
    TextEditor* m_TextEditor;

    bool m_showFlashModal = false;

    unsigned int* m_textureIDLampOff;
    unsigned int* m_textureIDLampOn;
};