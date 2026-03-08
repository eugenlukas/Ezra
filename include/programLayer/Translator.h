#pragma once
#include "FileLogger.h"

class Translator
{
public:
    void LoadLanguages()
    {
        for (const auto& entry : std::filesystem::directory_iterator("resources/Lang/"))
        {
            allLangCodes.push_back(entry.path().stem().string());
        }
    }
    bool LoadLanguage(const std::string& langCode)
    {
        messages.clear();
        currentLang = langCode;

        std::ifstream file("resources/Lang/" + langCode + ".txt");
        if (!file.is_open())
        {
            LOG("Could not open language file: " + langCode);
            return false;
        }

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#') continue; // skip comments
            auto sep = line.find('=');
            if (sep == std::string::npos) continue;

            std::string key = trim(line.substr(0, sep));
            std::string value = trim(line.substr(sep + 1));
            messages[key] = value;
        }

        return true;
    }

    std::string GetString(const std::string& key) const
    {
        auto it = messages.find(key);
        return it != messages.end() ? it->second : "[missing:" + key + "]";
    }
    const char* Get(const std::string& key) const
    {
        static std::string result;
        auto it = messages.find(key);
        if (it != messages.end())
        {
            result = it->second;
        }
        else
        {
            result = "[missing:" + key + "]";
        }
        return result.c_str();
    }
    const char* GetWithID(const std::string& key) const
    {
        static std::string result;
        auto it = messages.find(key);
        if (it != messages.end())
        {
            result = it->second + "###" + it->first;
        }
        else
        {
            result = "[missing:" + key + "]";
        }
        return result.c_str();
    }

    std::vector<std::string> GetAllLangCodes() { return allLangCodes; }

private:
	std::map<std::string, std::string> messages;
	std::string currentLang;
    std::vector<std::string> allLangCodes;

	std::string trim(const std::string& str)
	{
		auto start = str.find_first_not_of(" \t\r\n#");
		auto end = str.find_last_not_of(" \t\r\n");
		return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
	}
};