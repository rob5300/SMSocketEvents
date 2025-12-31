#pragma once
#include <string>
#include <filesystem>
#include <public/ITextParsers.h>
#include "../ext/nlohmann/json.hpp"

class Config
{
public:
    bool websockets = true;
    int port = 25570;
    bool secure = true;

    // File name of public key PEM file
    std::string public_key_file;

    //Parsed public key in PEM format
    std::string public_key_pem;

    int eventsPerFrame = 4;

    std::vector<std::string> tokens;

    bool LoadPEMPublicKey(const std::filesystem::path& configFolderPath);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config, websockets, port, secure, public_key_file, eventsPerFrame, tokens)