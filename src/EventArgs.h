#pragma once
#include <string>
#include <vector>
#include <map>
#include <smsdk_ext.h>

enum EventArgumentType
{
    EventArgumentType_NONE = 0,
    EventArgumentType_INT_32 = 1,
    EventArgumentType_STRING = 2,
    EventArgumentType_FLOAT = 3,
    EventArgumentType_BOOL = 4,
    EventArgumentType_ARGS = 5
};

class Argument
{
    public:
    std::string key;
    void* value;
    EventArgumentType type;

    Argument(const std::string& key, void* value, const EventArgumentType& type)
        : key(key), value(value), type(type)
    {}

    Argument(const std::string& key) : key(key), value(nullptr), type(EventArgumentType_NONE){};
};

class EventArgs
{
    public:
    ~EventArgs();
    bool ContainsKey(std::string& key);

    std::string* GetString(std::string& key);
    void SetString(std::string& key, std::string* value);

    int* GetInt(std::string& key);
    void SetInt(std::string& key, int32 value);

    float* GetFloat(std::string& key);
    void SetFloat(std::string& key, float value);

    bool* GetBool(std::string& key);
    void SetBool(std::string& key, bool value);

    private:
    std::vector<Argument*> arguments;
    std::map<std::string, Argument*> argumentMap;

    Argument* GetOrAddArgument(std::string& key);
    template <typename T>
    T* GetValueIfSameType(std::string& key, EventArgumentType type);
};

/// <summary>
/// Get the value pointer for a key if it exists and its type matches the provided type enum.
/// </summary>
template<typename T>
inline T* EventArgs::GetValueIfSameType(std::string& key, EventArgumentType type)
{
    if (ContainsKey(key))
    {
        Argument* arg = GetOrAddArgument(key);
        return arg->type == type ? static_cast<T*>(arg->value) : nullptr;
    }

    return nullptr;
}
