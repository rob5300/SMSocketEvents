#include "EventArgs.h"

using json = nlohmann::json;

EventArgs::EventArgs(const nlohmann::json& json)
{
    for (auto pair : json.items())
    {
        auto key = pair.key();
        auto value = pair.value();
        const auto type = value.type();
        switch (type)
        {
            case json::value_t::string:
                SetString(key, value.get<std::string>());
                break;

            case json::value_t::number_integer:
                SetInt(key, value.get<int32>());
                break;

            case json::value_t::number_float:
                SetFloat(key, value.get<float>());
                break;

            case json::value_t::boolean:
                SetBool(key, value.get<bool>());
                break;

            case json::value_t::object:
                SetEventArgs(key, new EventArgs(value));
        }
    }
}

bool EventArgs::ContainsKey(const std::string& key)
{
    return argumentMap.find(key) != argumentMap.end();
}

std::string* EventArgs::GetString(const std::string& key)
{
    return GetValueIfSameType<std::string>(GetKeyAsPath(key), EventArgumentType_STRING);
}

void EventArgs::SetString(const std::string& key, const std::string& value)
{
    Argument* arg = GetOrAddArgument(key);
    arg->type = EventArgumentType_STRING;
    arg->ClearValue();
    arg->value = new std::string(value);
}

int* EventArgs::GetInt(const std::string& key)
{
    return GetValueIfSameType<int32>(key, EventArgumentType_INT_32);
}

void EventArgs::SetInt(std::string& key, int32 value)
{
    Argument* arg = GetOrAddArgument(key);
    arg->type = EventArgumentType_INT_32;
    arg->ClearValue();
    arg->value = new int32(value);
}

float* EventArgs::GetFloat(const std::string& key)
{
    return GetValueIfSameType<float>(key, EventArgumentType_FLOAT);
}

void EventArgs::SetFloat(std::string& key, float value)
{
    Argument* arg = GetOrAddArgument(key);
    arg->type = EventArgumentType_FLOAT;
    arg->ClearValue();
    arg->value = new float(value);
}

bool* EventArgs::GetBool(const std::string& key)
{
    return GetValueIfSameType<bool>(key, EventArgumentType_BOOL);
}

void EventArgs::SetBool(std::string& key, bool value)
{
    Argument* arg = GetOrAddArgument(key);
    arg->type = EventArgumentType_BOOL;
    arg->ClearValue();
    arg->value = new bool(value);
}

EventArgs* EventArgs::GetEventArgs(const std::string& key)
{
    return GetValueIfSameType<EventArgs>(key, EventArgumentType_ARGS);
}

void EventArgs::SetEventArgs(const std::string& key, EventArgs* object)
{
    Argument* arg = GetOrAddArgument(key);
    arg->type = EventArgumentType_ARGS;
    arg->ClearValue();
    arg->value = object;
}

EventArgs::~EventArgs()
{
    //Delete all arguments
    for (auto arg : arguments)
    {
        delete arg;
    }
}

constexpr const char SEPERATOR = '/';
std::vector<std::string> EventArgs::GetKeyAsPath(const std::string& key)
{
    std::vector<std::string> path;

    size_t start = 0;
    for (size_t i = 0; i < key.length(); i++)
    {
        const auto c = key.at(i);
        if (c == SEPERATOR)
        {
            const size_t length = i - start;
            if (length > 0)
            {
                path.push_back(key.substr(start, length));
                start = i + 1;
            }
        }
    }
    
    if (start < key.length() - 1)
    {
        //Add from last start to end
        path.push_back(key.substr(start));
    }

    return path;
}

Argument* EventArgs::GetOrAddArgument(const std::string& key)
{
    Argument* arg = argumentMap[key];
    if (arg == nullptr)
    {
        arg = new Argument(key);
        argumentMap[key] = arg;
        arguments.push_back(arg);
    }
    return arg;
}

void Argument::ClearValue()
{
    if (value != nullptr)
    {
        delete value;
        value = nullptr;
        type = EventArgumentType_NONE;
    }
}

Argument::~Argument()
{
    if (value != nullptr)
    {
        delete value;
    }
}
