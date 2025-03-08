#include "EventArgs.h"
#include "Argument.h"
#include <iostream>
#include <string>

using json = nlohmann::json;

EventArgs::EventArgs()
{}

EventArgs::EventArgs(const nlohmann::json& json)
{
    for (auto pair : json.items())
    {
        const auto key = pair.key();
        const auto value = pair.value();
        const auto type = value.type();
        if (value.is_array() && value.size() != 0)
        {
            //Create array manually
            Argument arrayArg(key);
            const auto argType = JsonTypeToArgumentType(value[0].type());

            switch (argType)
            {
                case EventArgumentType_STRING:
                    arrayArg.SetValueAsArray<std::string>(value, argType);
                    break;

                case EventArgumentType_INT_32:
                    arrayArg.SetValueAsArray<int32>(value, argType);
                    break;

                case EventArgumentType_FLOAT:
                    arrayArg.SetValueAsArray<float>(value, argType);
                    break;

                case EventArgumentType_BOOL:
                    arrayArg.SetValueAsArray<bool>(value, argType);
                    break;

                case EventArgumentType_ARGS:
                    //Handle object array by creating many event args from each array element
                    {
                        auto eventArgsArray = std::shared_ptr<EventArgs[]>(new EventArgs[json.size()]);
                        for (size_t i = 0; i < json.size(); i++)
                        {
                            eventArgsArray[i] = EventArgs(json[i]);
                        }
                        arrayArg.type = EventArgumentType_ARGS;
                        arrayArg.isArray = true;
                        arrayArg.value = eventArgsArray;
                        arrayArg.length = json.size();
                    }
                    break;

                default:
                    std::cout << "EventArgs: Un-recognised type for var: " << key << std::endl;
                    break;
            }

            argumentMap[key] = arrayArg;
        }
        else
        {
            //Set single value
            switch (type)
            {
                case json::value_t::string:
                    SetString(key, value.get<std::string>());
                    break;

                case json::value_t::number_unsigned:
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
                    SetEventArgs(key, EventArgs(value));
                    break;

                default:
                    std::cout << "EventArgs: Un-recognised type for var: " << key << ". (" << static_cast<std::uint8_t>(type) << ")" << std::endl;
                    break;
            }
        }
    }
}

bool EventArgs::ContainsKey(const std::string& key)
{
    const auto keyPath = GetKeyAsPath(key);
    return GetArgumentWithPath(keyPath) != nullptr;
}

std::string* EventArgs::GetString(const std::string& key)
{
    return GetValueIfSameType<std::string>(GetKeyAsPath(key), EventArgumentType_STRING, false);
}

std::shared_ptr<std::string[]> EventArgs::GetStringArray(const std::string& key)
{
    return GetArrayValueIfSameType<std::string>(GetKeyAsPath(key), EventArgumentType_STRING);
}

void EventArgs::SetString(const std::string& key, const std::string& value)
{
    Argument* arg = GetOrAddArgument(key);
    arg->ClearValue();
    arg->type = EventArgumentType_STRING;
    arg->value = value;
}

int* EventArgs::GetInt(const std::string& key)
{
    return GetValueIfSameType<int32>(key, EventArgumentType_INT_32, false);
}

std::shared_ptr<int32[]> EventArgs::GetIntArray(const std::string& key)
{
    return GetArrayValueIfSameType<int32>(GetKeyAsPath(key), EventArgumentType_INT_32);
}

void EventArgs::SetInt(const std::string& key, int32 value)
{
    Argument* arg = GetOrAddArgument(key);
    arg->ClearValue();
    arg->type = EventArgumentType_INT_32;
    arg->value = value;
}

float* EventArgs::GetFloat(const std::string& key)
{
    return GetValueIfSameType<float>(key, EventArgumentType_FLOAT, false);
}

std::shared_ptr<float[]> EventArgs::GetFloatArray(const std::string& key)
{
    return GetArrayValueIfSameType<float>(GetKeyAsPath(key), EventArgumentType_FLOAT);
}

void EventArgs::SetFloat(const std::string& key, float value)
{
    Argument* arg = GetOrAddArgument(key);
    arg->ClearValue();
    arg->type = EventArgumentType_FLOAT;
    arg->value = value;
}

bool* EventArgs::GetBool(const std::string& key)
{
    return GetValueIfSameType<bool>(key, EventArgumentType_BOOL, false);
}

void EventArgs::SetBool(const std::string& key, bool value)
{
    Argument* arg = GetOrAddArgument(key);
    arg->ClearValue();
    arg->type = EventArgumentType_BOOL;
    arg->value = value;
}

EventArgs* EventArgs::GetEventArgs(const std::string& key)
{
    return GetValueIfSameType<EventArgs>(key, EventArgumentType_ARGS, false);
}

void EventArgs::SetEventArgs(const std::string& key, const EventArgs& object)
{
    Argument* arg = GetOrAddArgument(key);
    arg->ClearValue();
    arg->type = EventArgumentType_ARGS;
    arg->value = object;
}

std::any* EventArgs::GetValueRaw(const std::string& key)
{
    auto keyPath = GetKeyAsPath(key);
    Argument* arg = GetArgumentWithPath(keyPath);
    if (arg != nullptr)
    {
        return &arg->value;
    }

    return nullptr;
}

size_t EventArgs::GetArrayLength(const std::string& key)
{
    const Argument* arg = GetArgumentWithPath(GetKeyAsPath(key));
    if (arg != nullptr)
    {
        return arg->length;
    }
    
    return 0;
}

Argument* EventArgs::GetArgument(const std::string& key)
{
    if (ContainsKey(key))
    {
        return &argumentMap[key];
    }

    return nullptr;
}

EventArgumentType EventArgs::JsonTypeToArgumentType(const nlohmann::json::value_t valueType)
{
    switch (valueType)
    {
        case json::value_t::string:
            return EventArgumentType_STRING;

        case json::value_t::number_unsigned:
        case json::value_t::number_integer:
            return EventArgumentType_INT_32;

        case json::value_t::number_float:
            return EventArgumentType_FLOAT;

        case json::value_t::boolean:
            return EventArgumentType_BOOL;

        case json::value_t::object:
            return EventArgumentType_ARGS;

        default:
            return EventArgumentType_NONE;
    }
}

EventArgs::~EventArgs()
{
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
    
    if (start < key.length())
    {
        //Add from last start to end
        path.push_back(key.substr(start));
    }

    return path;
}

Argument* EventArgs::GetOrAddArgument(const std::string& key)
{
    if (!ContainsKey(key))
    {
        Argument arg(key);
        argumentMap[key] = arg;
    }

    return &argumentMap[key];
}

Argument* EventArgs::GetArgumentWithPath(const std::string& keyPath)
{
    const auto keyPathVec = GetKeyAsPath(keyPath);
    return GetArgumentWithPath(keyPathVec);
}

Argument* EventArgs::GetArgumentWithPath(const std::vector<std::string>& keyPath)
{
    //Go though event args tree then attempt to get desired value with last key
    EventArgs* args = this;
    for (size_t i = 0; i < keyPath.size() - 1; i++)
    {
        const auto key = keyPath[i];
        auto subEventArgs = args->GetEventArgs(key);
        if (subEventArgs != nullptr)
        {
            return nullptr;
        }
        else args = subEventArgs;
    }

    const auto key = keyPath[keyPath.size() - 1];
    if (args->argumentMap.find(key) != args->argumentMap.end())
    {
        return &args->argumentMap[key];
    }
    else return nullptr;
}
