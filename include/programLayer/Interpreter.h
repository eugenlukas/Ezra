#pragma once
#include <sstream>
#include <thread>
#include <atomic>
#include <chrono>

struct FlashRow
{
    bool command[4] = { 0 };
    bool address[4] = { 0 };
    bool value[4] = { 0 };
};

class Interpreter
{
public:
    void Run();
    void Stop();
	void Step();
    void Reset();

    void WriteFlashRowFromSave(std::string savedLine, int rowIndex);

    FlashRow(&GetFlash())[16]
    {
        return m_flash;
    }
    FlashRow(&GetRam())[8]
    {
        return m_ram;
    }
    FlashRow* GetRegisterA() { return &m_RegisterA; }
    FlashRow* GetRegisterB() { return &m_RegisterB; }
    FlashRow* GetRegisterX() { return &m_RegisterX; }
    int8_t GetCounter() { return m_RealCounter; }

    bool* GetZeFlag() { return &m_zeFlag; }
    bool* GetNeFlag() { return &m_neFlag; }
    bool* GetOvFlag() { return &m_ovFlag; }

    bool IsRunning() { return m_isRunning; }
    float* GetRunningSpeed() { return &m_runningSpeed; }

    const char* (&GetCommands())[16]
    {
        return m_commands;
    }

    bool* IntToBoolArray(int num, bool out[4]);
    int BoolArrayToInt(const bool arr[4]);

private:
    int Add4(int a, int b);
    int Sub4(int a, int b);
    int Mul4(int a, int b);
    int Div4(int a, int b);

    void UpdateFlags(int result, int rawResult);

private:
    FlashRow m_flash[16];
    FlashRow m_ram[8];
    FlashRow m_RegisterA;
    FlashRow m_RegisterB;
    FlashRow m_RegisterX;
    int8_t m_RealCounter = -1;

    bool m_zeFlag = false;
    bool m_neFlag = false;
    bool m_ovFlag = false;

    std::thread m_runThread;
    std::atomic<bool> m_isRunning{ false };
    float m_runningSpeed = 1.0f;

    const char* m_commands[16]{ "LDA const","LDA addr","LDB const", "LDB addr", "MUL", "DIV", "ADD", "SUB", "STX addr", "STA addr", "STB addr", "JMP stat", "JZE stat", "JOV stat", "JNE stat", "NOP" };
};