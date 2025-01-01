#pragma once
#include <string>
#include <vector>
#include <map>
#include <smsdk_ext.h>
#include <nlohmann/json.hpp>
#include <utility>
#include <iostream>
#include <exception>
#include "Argument.h"

class EventArgs
{
    public:
    EventArgs();
    explicit EventArgs(const nlohmann::json& json);
    ~EventArgs();

    bool ContainsKey(const std::string& key);

    std::string* GetString(const std::string& key);
    std::shared_ptr<std::string[]> GetStringArray(const std::string& key);
    void SetString(const std::string& key, const std::string& value);

    int* GetInt(const std::string& key);
    std::shared_ptr<int32[]> GetIntArray(const std::string& key);
    void SetInt(const std::string& key, int32 value);

    float* GetFloat(const std::string& key);
    std::shared_ptr<float[]> GetFloatArray(const std::string& key);
    void SetFloat(const std::string& key, float value);

    bool* GetBool(const std::string& key);
    void SetBool(const std::string& key, bool value);

    EventArgs* GetEventArgs(const std::string& key);
    void SetEventArgs(const std::string& key, const EventArgs& object);

    std::any* GetValueRaw(const std::string& key);

    size_t GetArrayLength(const std::string& key);

    Argument* GetArgument(const std::string& key);
    Argument* GetArgumentWithPath(const std::string& keyPath);
    Argument* GetArgumentWithPath(const std::vector<std::string>& keyPath);

    private:
    static EventArgumentType JsonTypeToArgumentType(const nlohmann::json::value_t valueType);
    std::map<std::string, Argument> argumentMap;

    std::vector<std::string> GetKeyAsPath(const std::string& key);
    Argument* GetOrAddArgument(const std::string& key);

    template <typename T>
    T* GetValueIfSameType(const std::vector<std::string>& keyPath, EventArgumentType type, bool isArray);

    template <typename T>
    T* GetValueIfSameType(const std::string& key, EventArgumentType type, bool isArray);

    template <typename T>
    std::shared_ptr<T[]> GetArrayValueIfSameType(const std::vector<std::string>& keyPath, EventArgumentType type);
};

template<typename T>
inline T* EventArgs::GetValueIfSameType(const std::vector<std::string>& keyPath, EventArgumentType type, bool isArray)
{
    EventArgs* lastEventArgs = this;
    Argument* lastArg = nullptr;

    //Loop over key path either getting next event args or argument.
    //If we cannot go deeper but have an array arg, try to get array element, else the arg value is returned as normal.
    for (size_t i = 0; i < keyPath.size(); i++)
    {
        const auto key = keyPath[i];
        //If we have event args here, get next key
        if (lastEventArgs != nullptr)
        {
            Argument* arg = lastEventArgs->GetArgument(key);
            if (arg != nullptr)
            {
                lastArg = arg;
                lastEventArgs = nullptr;
                if (arg->type == EventArgumentType_ARGS)
                {
                    lastEventArgs = lastArg->GetValue<EventArgs>();
                }
            }
        }
        else if (lastArg != nullptr && lastArg->isArray)
        {
            //Try to parse key as index and get index value.
            //If event args array, get event args and continue, else get value and return early!
            try
            {
                int index = std::stoi(key);
                if (lastArg->type == EventArgumentType_ARGS)
                {
                    lastEventArgs = lastArg->GetArrayValueElement<EventArgs>(index);
                    lastArg = nullptr;
                }
                else if(lastArg->type == type)
                {
                    //Return array value at index
                    return lastArg->GetArrayValueElement<T>(index);
                }
            }
            catch (const std::exception e)
            {
                std::cerr << "GetValueIfSameType error:" << e.what() << std::endl;
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }

    //If the last arg matches the type and array, return its value
    if (lastArg != nullptr && lastArg->type == type && lastArg->isArray == isArray)
    {
        return lastArg->GetValue<T>();
    }

    return nullptr;
}

/// <summary>
/// Get the value pointer for a key if it exists and its type matches the provided type enum.
/// </summary>
template<typename T>
inline T* EventArgs::GetValueIfSameType(const std::string& key, EventArgumentType type, bool isArray)
{
    return EventArgs::GetValueIfSameType<T>(GetKeyAsPath(key), type, isArray);
}

template<typename T>
inline std::shared_ptr<T[]> EventArgs::GetArrayValueIfSameType(const std::vector<std::string>& keyPath, EventArgumentType type)
{
    Argument* arg = GetArgumentWithPath(keyPath);
    if (arg != nullptr && arg->isArray && arg->type == type)
    {
        return arg->GetValueAsArray<T>();
    }

    return nullptr;
}
