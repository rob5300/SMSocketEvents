#include "Config.h"
#include <cstring>
#include <fstream>
#include <sstream>

bool Config::LoadPEMPublicKey(const std::filesystem::path& configFolderPath)
{
    std::ifstream keystream(configFolderPath / public_key_file);
    if (keystream.is_open())
    {
        std::stringstream key;
        key << keystream.rdbuf();
        this->public_key_pem = key.str();
        return true;
    }
    else
    {
        secure = false;
    }

    return false;
}

unsigned int Config::GetTextParserVersion1()
{
    return 0;
}

bool Config::ReadINI_NewSection(const char* section, bool invalid_tokens, bool close_bracket, bool extra_tokens, unsigned int* curtok)
{
    return true;
}

bool Config::ReadINI_KeyValue(const char* key, const char* value, bool invalid_tokens, bool equal_token, bool quotes, unsigned int* curtok)
{
    if (strcmp(key, "port") == 0)
    {
        port = std::stoi(value);
    }
    else if (strcmp(key, "public_key_file") == 0)
    {
        public_key_file = std::string(value);
    }
    else if (strcmp(key, "eventsPerFrame") == 0)
    {
        eventsPerFrame = std::max(std::stoi(value), 1);
    }
    return true;
}

bool Config::ReadINI_RawLine(const char* line, unsigned int* curtok)
{
    return true;
}
