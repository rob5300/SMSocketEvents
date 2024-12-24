#pragma once
#include <smsdk_ext.h>
#include "EventArgs.h"

class EventArgsHandler : public SourceMod::IHandleTypeDispatch
{
public:
    void OnHandleDestroy(HandleType_t type, void *object)
    {
        EventArgs* eventArgs = static_cast<EventArgs*>(object);
        delete eventArgs;
    }
};

extern SourceMod::HandleType_t EventArgsHandleType;
extern EventArgsHandler eventArgsHandler;

EventArgs* GetEventArgsFromHandle(SourceMod::Handle_t& handle, IPluginContext* pluginContext);
cell_t smn_EventArgs_GetString(SourcePawn::IPluginContext* pluginContext, const cell_t* params);
cell_t smn_EventArgs_GetInt(SourcePawn::IPluginContext* pluginContext, const cell_t* params);
cell_t smn_EventArgs_GetFloat(SourcePawn::IPluginContext* pluginContext, const cell_t* params);
cell_t smn_EventArgs_GetBool(SourcePawn::IPluginContext* pluginContext, const cell_t* params);
cell_t smn_EventArgs_ContainsKey(SourcePawn::IPluginContext* pluginContext, const cell_t* params);
cell_t smn_EventArgs_IsArray(SourcePawn::IPluginContext* pluginContext, const cell_t* params);
cell_t smn_EventArgs_GetArrayLength(SourcePawn::IPluginContext* pluginContext, const cell_t* params);

/*
template <typename T>
bool CopyArrayToPluginArray(T* fromArray, size_t size, SourcePawn::IPluginContext* pluginContext, const cell_t* params);

template<typename T>
inline bool CopyArrayToPluginArray(T* fromArray, size_t size, SourcePawn::IPluginContext* pluginContext, const cell_t* params)
{
    if (fromArray != nullptr)
    {
        cell_t spArraySize = params[4];

        if (spArraySize < size)
        {
            //❗Throw error: Array in sp is too small!
            pluginContext->ThrowNativeError("Array needs to be '%d' or larger", value.second);
            return false;
        }
        
        cell_t* pluginArray;
        pluginContext->LocalToPhysAddr(params[3], &pluginArray);
        for (cell_t i = 0; i < spArraySize; i++)
        {
            pluginArray[i] = fromArray[i];
        }

        return true;
    }

    return false;
}
*/
