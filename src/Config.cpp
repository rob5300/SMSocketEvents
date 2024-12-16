#include "Config.h"

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
    else if (strcmp(key, "public_key") == 0)
    {
        public_key = std::string(value);
    }
    return true;
}

bool Config::ReadINI_RawLine(const char* line, unsigned int* curtok)
{
    return true;
}
