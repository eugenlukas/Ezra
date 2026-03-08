#include "SubWindows.h"
#include "imgui.h"
#include <string>
#include <FileLogger.h>

void SubWindows::Render()
{
    RenderFlash();
    RenderALU();
    RenderCounter();
    RenderRam();
    RenderOutput();
}

void SubWindows::RenderFlash()
{
    ImGui::Begin("Flash");

    if (ImGui::BeginTable("FlashTable", 4, ImGuiTableFlags_Borders))
    {
        ImGui::TableSetupColumn("Index");
        ImGui::TableSetupColumn("Command");
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        for (int row = 0; row < 16; row++)
        {
            ImGui::TableNextRow();

            // Row index
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", row);

            // highlight logic
            if (row == m_interpreter->GetCounter())
            {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(255, 255, 0, 100));
            }

            // Command bits
            ImGui::TableSetColumnIndex(1);

            for (int i = 0; i < 4; i++)
            {
                ImGui::PushID(row * 10 + i);
                BitButton("cmd", m_interpreter->GetFlash()[row].command[i], true, row);
                ImGui::SameLine();
                ImGui::PopID();
            }

            // Address bits
            ImGui::TableSetColumnIndex(2);
            for (int i = 0; i < 4; i++)
            {
                ImGui::PushID(row * 20 + i);
                BitButton("addr", m_interpreter->GetFlash()[row].address[i], true, row);
                ImGui::SameLine();
                ImGui::PopID();
            }

            // Value bits
            ImGui::TableSetColumnIndex(3);
            for (int i = 0; i < 4; i++)
            {
                ImGui::PushID(row * 30 + i);
                BitButton("val", m_interpreter->GetFlash()[row].value[i], true, row);
                ImGui::SameLine();
                ImGui::PopID();
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void SubWindows::RenderALU()
{
    ImGui::Begin("ALU");

    // Register A and B
    if (ImGui::BeginTable("Register AB", 2, ImGuiTableFlags_Borders))
    {
        ImGui::TableSetupColumn("Register A");
        ImGui::TableSetupColumn("Register B");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        for (int i = 0; i < 4; i++)
        {
            ImGui::PushID(("Register_A_" + std::to_string(i)).c_str());
            BitButton("val", m_interpreter->GetRegisterA()->value[i], false);
            ImGui::SameLine();
            ImGui::PopID();
        }

        ImGui::TableSetColumnIndex(1);
        for (int i = 0; i < 4; i++)
        {
            ImGui::PushID(("Register_B_" + std::to_string(i)).c_str());
            BitButton("val", m_interpreter->GetRegisterB()->value[i], false);
            ImGui::SameLine();
            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    // Padding
    ImGui::Dummy(ImVec2(0.0f, 20.0f));

    // Register X
    if (ImGui::BeginTable("Register X", 1, ImGuiTableFlags_Borders))
    {
        ImGui::TableSetupColumn("Register X");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);

        for (int i = 0; i < 4; i++)
        {
            ImGui::PushID(i);
            BitButton("val", m_interpreter->GetRegisterX()->value[i], false);
            ImGui::SameLine();
            ImGui::PopID();
        }

        ImGui::EndTable();
    }

#pragma region Flags

    // Padding
    float heigth = (ImGui::GetWindowHeight() - ImGui::GetCursorPosY()) - 50;
    ImGui::Dummy(ImVec2(0.0f, heigth));

    // Center checkboxes
    // Calculate checkbox item width
    // The box itself is roughly the size of a line height (FrameHeight)
    float window_width = ImGui::GetContentRegionAvail().x;
    float checkbox_width = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x + ImGui::CalcTextSize("ze").x;
    ImGui::SetCursorPosX((window_width - checkbox_width * 3) * 0.5f);

    ImGui::Checkbox("ze", m_interpreter->GetZeFlag());
    ImGui::SameLine();
    ImGui::Checkbox("ne", m_interpreter->GetNeFlag());
    ImGui::SameLine();
    ImGui::Checkbox("ov", m_interpreter->GetOvFlag());
#pragma endregion

    ImGui::End();
}

void SubWindows::RenderCounter()
{
    ImGui::Begin("Counter");

    if (ImGui::BeginTable("Counter", 1, ImGuiTableFlags_Borders))
    {
        ImGui::TableSetupColumn("");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);

        FlashRow row;
        if (m_interpreter->GetCounter() == -1)
            m_interpreter->IntToBoolArray(0, row.value);
        else
            m_interpreter->IntToBoolArray(m_interpreter->GetCounter(), row.value);

        for (int i = 0; i < 4; i++)
        {
            ImGui::PushID(i);
            BitButton("val", row.value[i], false);
            ImGui::SameLine();
            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void SubWindows::RenderRam()
{
    ImGui::Begin("RAM");

    if (ImGui::BeginTable("FlashTable", 2, ImGuiTableFlags_Borders))
    {
        ImGui::TableSetupColumn("Index");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        for (int row = 0; row < 8; row++)
        {
            ImGui::TableNextRow();

            // Row index
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", row);

            // Value bits
            ImGui::TableSetColumnIndex(1);
            for (int i = 0; i < 4; i++)
            {
                ImGui::PushID(row * 30 + i);
                BitButton("val", m_interpreter->GetRam()[row].value[i], false);
                ImGui::SameLine();
                ImGui::PopID();
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void SubWindows::RenderOutput()
{
    ImGui::Begin("Output");

    float imageWith = (ImGui::GetWindowWidth() / 4) - 10;
    unsigned int lamp1ImageId = m_interpreter->GetRam()->value[0] == 1 ? *m_textureIDLampOn : *m_textureIDLampOff;
    unsigned int lamp2ImageId = m_interpreter->GetRam()->value[1] == 1 ? *m_textureIDLampOn : *m_textureIDLampOff;
    unsigned int lamp3ImageId = m_interpreter->GetRam()->value[2] == 1 ? *m_textureIDLampOn : *m_textureIDLampOff;
    unsigned int lamp4ImageId = m_interpreter->GetRam()->value[3] == 1 ? *m_textureIDLampOn : *m_textureIDLampOff;

    ImGui::Image(lamp1ImageId, ImVec2(imageWith, imageWith));
    ImGui::SameLine();
    ImGui::Image(lamp2ImageId, ImVec2(imageWith, imageWith));
    ImGui::SameLine();
    ImGui::Image(lamp3ImageId, ImVec2(imageWith, imageWith));
    ImGui::SameLine();
    ImGui::Image(lamp4ImageId, ImVec2(imageWith, imageWith));

    ImGui::End();
}

bool SubWindows::BitButton(const char* id, bool& bit, bool isFlashRow, uint8_t flashRowIndex)
{
    ImGui::PushID(id);

    if (bit)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1));
    else
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1));

    bool pressed = ImGui::Button(bit ? "1" : "0", ImVec2(22, 22));

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && isFlashRow)
    {
        ImGui::SetNextWindowPos(ImGui::GetMousePos());
        ImGui::OpenPopup("FlashModal");
    }

    if (ImGui::BeginPopupModal("FlashModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        RenderFlashModal(flashRowIndex);
        ImGui::EndPopup();
    }

    if (pressed)
        bit = !bit;

    ImGui::PopStyleColor();
    ImGui::PopID();

    return pressed;
}

void SubWindows::RenderFlashModal(uint8_t rowIndex)
{
    FlashRow* row = &m_interpreter->GetFlash()[rowIndex];

    // Static so values persist while modal is open
    static int Selecteditem = 0;
    static int value = 0;
    static FlashRow* currentRow = nullptr;

    // If the modal is opened for a different row, reset values
    if (currentRow != row)
    {
        Selecteditem = m_interpreter->BoolArrayToInt(row->command);
        value = m_interpreter->BoolArrayToInt(row->value) == 0 ? m_interpreter->BoolArrayToInt(row->address) : m_interpreter->BoolArrayToInt(row->value);
        currentRow = row;
    }

    if (ImGui::Combo("Command", &Selecteditem, m_interpreter->GetCommands(), IM_ARRAYSIZE(m_interpreter->GetCommands())))
    {
        // Here event is fired
    }

    ImGui::SameLine();

    bool disableValue = false;
    if (Selecteditem == 4 || Selecteditem == 5 || Selecteditem == 6 || Selecteditem == 7 || Selecteditem == 15)
        disableValue = true;
    ImGui::BeginDisabled(disableValue);
    if (ImGui::InputInt("Value", &value))
    {
        if (value < 0)   value = 0;   // Min
        if (value > 15) value = 15;   // Max
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    if (ImGui::Button("Ok"))
    {
        WriteChnagesToFlash(Selecteditem, rowIndex, value);

        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button("Close"))
        ImGui::CloseCurrentPopup();
}

void SubWindows::WriteChnagesToFlash(int selectedItem, int rowIndex, int rowValue)
{
    FlashRow* row = &m_interpreter->GetFlash()[rowIndex];
    bool writeNoValue = false;

    // Write in flash table
    switch (selectedItem)
    {
    case 0:
    case 2:
        m_interpreter->IntToBoolArray(selectedItem, row->command);
        m_interpreter->IntToBoolArray(0, row->address);
        m_interpreter->IntToBoolArray(rowValue, row->value);
        writeNoValue = false;
        break;
    case 1:
    case 3:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
        m_interpreter->IntToBoolArray(selectedItem, row->command);
        m_interpreter->IntToBoolArray(rowValue, row->address);
        m_interpreter->IntToBoolArray(0, row->value);
        writeNoValue = false;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 15:
        m_interpreter->IntToBoolArray(selectedItem, row->command);
        m_interpreter->IntToBoolArray(0, row->address);
        m_interpreter->IntToBoolArray(0, row->value);
        writeNoValue = true;
        break;
    }

    // Write in text editor
    std::string insertText(m_interpreter->GetCommands()[selectedItem]);
    if (!writeNoValue)
        insertText += " " + std::to_string(rowValue);

    m_TextEditor->SetCursorPosition(TextEditor::Coordinates(rowIndex, 0));
    m_TextEditor->SetReadOnly(false);

    if (m_TextEditor->GetCurrentLineText() != "")
    {
        m_TextEditor->SetSelection(TextEditor::Coordinates(rowIndex, 0), TextEditor::Coordinates(rowIndex, m_TextEditor->GetCurrentLineText().size()));
        m_TextEditor->Delete();
    }
    
    m_TextEditor->InsertText(insertText, true);
    m_TextEditor->SetReadOnly(true);
}
