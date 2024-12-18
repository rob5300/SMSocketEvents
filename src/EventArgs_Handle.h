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
