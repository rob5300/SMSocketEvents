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
