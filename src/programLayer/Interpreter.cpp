#include "Interpreter.h"
#include <algorithm>
#include <vector>
#include <FileLogger.h>

void Interpreter::Run()
{
    if (m_isRunning)
        return;

    m_isRunning = true;
    m_runThread = std::thread([this]()
        {
            while (m_isRunning)
            {
                Step();

                // Sleep for m_runningSpeed seconds
                std::this_thread::sleep_for(std::chrono::duration<float>(m_runningSpeed));
            }
        });
}

void Interpreter::Stop()
{
    m_isRunning = false;
    if (m_runThread.joinable())
        m_runThread.join(); // Wait for it to finish cleanly
}

void Interpreter::Step()
{
    // Increment
    if (m_RealCounter == 15)
        m_RealCounter = 0;
    else
        m_RealCounter++;

    // Get Command
    int rowIndex = m_RealCounter;
    int addressIndex = BoolArrayToInt(m_flash[rowIndex].address);
    int selectedCommandIndex = BoolArrayToInt(m_flash[rowIndex].command);

    // Handle Command
    int RegANum = BoolArrayToInt(m_RegisterA.value);
    int RegBNum = BoolArrayToInt(m_RegisterB.value);
    int result = 0;
    FlashRow jumpRow;

    switch (selectedCommandIndex)
    {
    case 0: // LDA const
        std::copy(std::begin(m_flash[rowIndex].value), std::end(m_flash[rowIndex].value), std::begin(m_RegisterA.value));
        break;
    case 1: // LDA addr
        std::copy(std::begin(m_ram[addressIndex].value), std::end(m_ram[addressIndex].value), std::begin(m_RegisterA.value));
        break;
    case 2: // LDB const
        std::copy(std::begin(m_flash[rowIndex].value), std::end(m_flash[rowIndex].value), std::begin(m_RegisterB.value));
        break;
    case 3: // LDB addr
        std::copy(std::begin(m_ram[addressIndex].value), std::end(m_ram[addressIndex].value), std::begin(m_RegisterB.value));
        break;
    case 4: // MUL
        result = Mul4(RegANum, RegBNum);
        IntToBoolArray(result, m_RegisterX.value);
        break;
    case 5: // DIV
        result = Div4(RegANum, RegBNum);
        IntToBoolArray(result, m_RegisterX.value);
        break;
    case 6: // ADD
        result = Add4(RegANum, RegBNum);
        IntToBoolArray(result, m_RegisterX.value);
        break;
    case 7: // SUB
        result = Sub4(RegANum, RegBNum);
        IntToBoolArray(result, m_RegisterX.value);
        break;
    case 8: // STX addr
        std::copy(std::begin(m_RegisterX.value), std::end(m_RegisterX.value), std::begin(m_ram[addressIndex].value));
        break;
    case 9: // STA addr
        std::copy(std::begin(m_RegisterA.value), std::end(m_RegisterA.value), std::begin(m_ram[addressIndex].value));
        break;
    case 10: // STB addr
        std::copy(std::begin(m_RegisterB.value), std::end(m_RegisterB.value), std::begin(m_ram[addressIndex].value));
    case 11: // JMP stat
        IntToBoolArray(BoolArrayToInt(m_flash[rowIndex].address), jumpRow.value); // unconditional jump
        m_RealCounter = BoolArrayToInt(jumpRow.value);
        return; // skip counter increment
        break;
    case 12: // JZE stat
        if (m_zeFlag)
        {
            IntToBoolArray(BoolArrayToInt(m_flash[rowIndex].address), jumpRow.value);
            m_RealCounter = BoolArrayToInt(jumpRow.value);
            return; // skip counter increment
        }
        break;
    case 13: // JOV stat
        if (m_ovFlag)
        {
            IntToBoolArray(BoolArrayToInt(m_flash[rowIndex].address), jumpRow.value);
            m_RealCounter = BoolArrayToInt(jumpRow.value);
            return; // skip counter increment
        }
        break;
    case 14: // JNE stat
        if (m_neFlag)
        {
            IntToBoolArray(BoolArrayToInt(m_flash[rowIndex].address), jumpRow.value);
            m_RealCounter = BoolArrayToInt(jumpRow.value);
            return; // skip counter increment
        }
        break;
    case 15: // NOP
        break;
    }
}

void Interpreter::Reset()
{
    m_RealCounter = -1;

    IntToBoolArray(0, m_RegisterA.value);
    IntToBoolArray(0, m_RegisterB.value);
    IntToBoolArray(0, m_RegisterX.value);

    for (auto& row : m_ram)
        IntToBoolArray(0, row.value);

    m_zeFlag = false;
    m_neFlag = false;
    m_ovFlag = false;
}

int Interpreter::Add4(int a, int b)
{
    int raw = a + b;
    int result = raw & 0xF;
    UpdateFlags(result, raw);
    return result;
}

int Interpreter::Sub4(int a, int b)
{
    int raw = a - b;
    int result = raw & 0xF;
    UpdateFlags(result, raw);
    return result;
}

int Interpreter::Mul4(int a, int b)
{
    int raw = a * b;
    int result = raw & 0xF;
    UpdateFlags(result, raw);
    return result;
}

int Interpreter::Div4(int a, int b)
{
    if (b == 0)
    {
        LOG("Division by zero!");
        m_zeFlag = true;
        m_neFlag = false;
        m_ovFlag = true;
        return 0;
    }
    int raw = a / b;
    int result = raw & 0xF;
    UpdateFlags(result, raw);
    return result;
}

void Interpreter::UpdateFlags(int result, int rawResult)
{
    m_zeFlag = (result == 0);
    m_neFlag = rawResult < 0 ? true : false;
    m_ovFlag = (rawResult > 15 || rawResult < 0);
}

void Interpreter::WriteFlashRowFromSave(std::string savedLine, int rowIndex)
{
    std::stringstream ss(savedLine);

    std::vector<std::string> words{ std::istream_iterator<std::string>{ss}, std::istream_iterator<std::string>{} };

    if (words.size() == 0)
        return;

    std::string commandToFind = words.size() > 1 ? words[0] + " " + words[1] : words[0];

    int selectedCommand = 0;
    auto it = std::find(std::begin(m_commands), std::end(m_commands), commandToFind);
    if (it != std::end(m_commands))
        selectedCommand = std::distance(std::begin(m_commands), it);
    else
    {
        LOG("Line command could not be found: '" + commandToFind + "'");
        return;
    }

    IntToBoolArray(selectedCommand, m_flash[rowIndex].command);

    if (words.size() > 1)
    {
        if (words[1] == "const")
        {
            IntToBoolArray(0, m_flash[rowIndex].address);
            int value = std::stoi(words[2]);
            IntToBoolArray(value, m_flash[rowIndex].value);
        }
        else if (words[1] == "addr" || words[1] == "stat")
        {
            int address = std::stoi(words[2]);
            IntToBoolArray(address, m_flash[rowIndex].address);
            IntToBoolArray(0, m_flash[rowIndex].value);
        }
        else
            LOG("Line command could not be parsed: '" + commandToFind + "'. In: ' " + words[1] + "'");
    }
    else
    {
        IntToBoolArray(0, m_flash[rowIndex].address);
       IntToBoolArray(0, m_flash[rowIndex].value);
    }
}

bool* Interpreter::IntToBoolArray(int num, bool out[4])
{
    if (num < 0 || num > 15) return nullptr;

    for (int i = 0; i < 4; ++i)
    {
        out[3 - i] = (num >> i) & 1;
    }

    return out;
}

int Interpreter::BoolArrayToInt(const bool arr[4])
{
    int result = 0;
    for (int i = 0; i < 4; ++i)
    {
        result |= arr[i] << (3 - i);
    }
    return result;
}
