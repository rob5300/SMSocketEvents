#pragma once
#include <smsdk_ext.h>
#include "EventArgs.h"

class EventArgsHandler : public SourceMod::IHandleTypeDispatch
{
public:
    void OnHandleDestroy(HandleType_t type, void *object)
    {
        EventArgs* eventArgs = reinterpret_cast<EventArgs*>(object);
        delete eventArgs;
    }
};

extern SourceMod::HandleType_t EventArgsHandleType;
extern EventArgsHandler eventArgsHandler;

EventArgs* GetEventArgsFromHandle(SourceMod::Handle_t& handle, IPluginContext* pluginContext);
cell_t smn_EventArgs_GetString(SourcePawn::IPluginContext* pluginContext, const cell_t* params);
//static cell_t smn_EventArgs_GetInt(IPluginContext* pluginContext, const cell_t* params);
