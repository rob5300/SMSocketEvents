#pragma once
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include <utility>
#include <exception>
#include "ArgumentException.h"

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

    // Key of argument, empty string if unset.
    std::string key;

    // Argument value. Can be std::string, int32, bool, float, EventArgs or shared_ptr<T[]>
    std::any value;

    /// <summary>
    /// Type of the value pointer (if none then value is nullptr)
    /// </summary>
    EventArgumentType type;

    /// <summary>
    /// Is this an array? (Stored via shared_ptr<T[]>)
    /// </summary>
    bool isArray;

    // Length of array, else 0.
    size_t length = 0;

    Argument()
    {}

    Argument(const std::string& key, void* value, const EventArgumentType& type)
        : key(key), value(value), type(type), isArray(false)
    {
    }

    Argument(const std::string& key) : key(key), value(nullptr), type(EventArgumentType_NONE), isArray(false)
    {
    };

    void ClearValue();

    /// <summary>
    /// Parse json value as an array of type T. Stored as std::shared_ptr<T[]>
    /// </summary>
    template<typename T>
    void SetValueAsArray(const nlohmann::json& json, const EventArgumentType type);

    template<typename T>
    T* GetValue();

    template<typename T>
    std::shared_ptr<T[]> GetValueAsArray();

    template<typename T>
    T* GetArrayValueElement(size_t index);
};

template<typename T>
inline void Argument::SetValueAsArray(const nlohmann::json& json, const EventArgumentType type)
{
    if (json.is_array())
    {
        //Store new array in shared pointer so any storage is better
        std::shared_ptr<T[]> valueArray = std::shared_ptr<T[]>(new T[json.size()]);
        for (size_t i = 0; i < json.size(); i++)
        {
            T item = json[i].get<T>();
            valueArray.get()[i] = item;
        }
        this->type = type;
        isArray = true;
        value = valueArray;
        length = json.size();
    }
    else
    {
        throw ArgumentException("Json value argument is not an array");
    }
}

template<typename T>
inline T* Argument::GetValue()
{
    if (this->value.has_value() && !this->isArray)
    {
        T* ptr = std::any_cast<T>(&value);
        return ptr;
    }

    return nullptr;
}

template<typename T>
inline std::shared_ptr<T[]> Argument::GetValueAsArray()
{
    if (this->value.has_value() && this->isArray)
    {
        return std::any_cast<std::shared_ptr<T[]>>(value);
    }

    return nullptr;
}

template<typename T>
inline T* Argument::GetArrayValueElement(size_t index)
{
    //Return nullptr if out of range or no value
    if((index < 0 || index >= length) && value.has_value()) return nullptr;

    std::shared_ptr<T[]> arraySharedPtr = GetValueAsArray<T>();
    return &arraySharedPtr.get()[index];
}
