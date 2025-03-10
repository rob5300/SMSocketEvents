#pragma once
#include <string>
#include <filesystem>
#include <public/ITextParsers.h>

class Config : public SourceMod::ITextListener_INI
{
public:
    int port = 25570;
    bool secure = true;

    // File name of public key PEM file
    std::string public_key_file;

    //Parsed public key in PEM format
    std::string public_key_pem;

    int eventsPerFrame = 4;

    bool LoadPEMPublicKey(const std::filesystem::path& configFolderPath);

    virtual unsigned int GetTextParserVersion1();
    virtual bool ReadINI_NewSection (const char *section, bool invalid_tokens, bool close_bracket, bool extra_tokens, unsigned int *curtok);
    virtual bool ReadINI_KeyValue (const char *key, const char *value, bool invalid_tokens, bool equal_token, bool quotes, unsigned int *curtok);
    virtual bool ReadINI_RawLine (const char *line, unsigned int *curtok);
};
