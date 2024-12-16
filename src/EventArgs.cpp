#include "EventArgs.h"

bool EventArgs::ContainsKey(std::string& key)
{
    return argumentMap.find(key) != argumentMap.end();
}

std::string* EventArgs::GetString(std::string& key)
{
    return GetValueIfSameType<std::string>(key, EventArgumentType_STRING);
}

void EventArgs::SetString(std::string& key, std::string* value)
{
    Argument* arg = GetOrAddArgument(key);
    arg->type = EventArgumentType_STRING;
    if (arg->value != nullptr)
    {
        delete arg->value;
    }
    arg->value = new std::string(*value);
}

int* EventArgs::GetInt(std::string& key)
{
    return GetValueIfSameType<int32>(key, EventArgumentType_INT_32);
}

void EventArgs::SetInt(std::string& key, int32 value)
{
    Argument* arg = GetOrAddArgument(key);
    arg->type = EventArgumentType_INT_32;
    if (arg->value != nullptr)
    {
        delete arg->value;
    }
    arg->value = new int32(value);
}

float* EventArgs::GetFloat(std::string& key)
{
    return GetValueIfSameType<float>(key, EventArgumentType_FLOAT);
}

void EventArgs::SetFloat(std::string& key, float value)
{
    Argument* arg = GetOrAddArgument(key);
    arg->type = EventArgumentType_FLOAT;
    if (arg->value != nullptr)
    {
        delete arg->value;
    }
    arg->value = new float(value);
}

bool* EventArgs::GetBool(std::string& key)
{
    return GetValueIfSameType<bool>(key, EventArgumentType_BOOL);
}

void EventArgs::SetBool(std::string& key, bool value)
{
    Argument* arg = GetOrAddArgument(key);
    arg->type = EventArgumentType_BOOL;
    if (arg->value != nullptr)
    {
        delete arg->value;
    }
    arg->value = new bool(value);
}

Argument* EventArgs::GetOrAddArgument(std::string& key)
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

EventArgs::~EventArgs()
{
    //TODO: Delete all argument values!

}
