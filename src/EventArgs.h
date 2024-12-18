#pragma once
#include <string>
#include <vector>
#include <map>
#include <smsdk_ext.h>
#include <nlohmann/json.hpp>

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

    void ClearValue();

    ~Argument();
};

class EventArgs
{
    public:
    explicit EventArgs(const nlohmann::json& json);
    ~EventArgs();
    bool ContainsKey(const std::string& key);

    std::string* GetString(const std::string& key);
    void SetString(const std::string& key, const std::string& value);

    int* GetInt(const std::string& key);
    void SetInt(std::string& key, int32 value);

    float* GetFloat(const std::string& key);
    void SetFloat(std::string& key, float value);

    bool* GetBool(const std::string& key);
    void SetBool(std::string& key, bool value);

    EventArgs* GetEventArgs(const std::string& key);
    void SetEventArgs(const std::string& key, EventArgs* object);

    private:
    std::vector<Argument*> arguments;
    std::map<std::string, Argument*> argumentMap;

    std::vector<std::string> GetKeyAsPath(const std::string& key);
    Argument* GetOrAddArgument(const std::string& key);

    Argument* GetArgumentWithPath(const std::vector<std::string>& keyPath);

    template <typename T>
    T* GetValueIfSameType(const std::vector<std::string>& keyPath, EventArgumentType type);

    template <typename T>
    T* GetValueIfSameType(const std::string& key, EventArgumentType type);
};

template<typename T>
inline T* EventArgs::GetValueIfSameType(const std::vector<std::string>& keyPath, EventArgumentType type)
{
    //Go though event args tree then attempt to get desired value with last key
    EventArgs* args = this;
    for (size_t i = 0; i < keyPath.size() - 1; i++)
    {
        const auto key = keyPath[i];
        auto subEventArgs = args->GetEventArgs(key);
        if (subEventArgs == nullptr)
        {
            return nullptr;
        }
        else args = subEventArgs;
    }

    if (args != nullptr)
    {
        //Get value with final key
        return args->GetValueIfSameType<T>(keyPath[keyPath.size() - 1], type);
    }

    return nullptr;
}

/// <summary>
/// Get the value pointer for a key if it exists and its type matches the provided type enum.
/// </summary>
template<typename T>
inline T* EventArgs::GetValueIfSameType(const std::string& key, EventArgumentType type)
{
    if (ContainsKey(key))
    {
        Argument* arg = GetOrAddArgument(key);
        return arg->type == type ? static_cast<T*>(arg->value) : nullptr;
    }

    return nullptr;
}
