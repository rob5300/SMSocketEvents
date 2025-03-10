﻿#include <string>
#include "EventArgs_Handle.h"

using namespace SourceMod;

HandleType_t EventArgsHandleType;
EventArgsHandler eventArgsHandler;

EventArgs* GetEventArgsFromHandle(SourceMod::Handle_t& handle, IPluginContext* pluginContext)
{
    if (EventArgsHandleType == 0)
    {
        pluginContext->ThrowNativeError("EventArgs Handle Type was not created by extension");
        return nullptr;
    }

    HandleError handleError;
    HandleSecurity sec;
    sec.pIdentity = myself->GetIdentity();
    EventArgs* eventArgs;
    if ((handleError = handlesys->ReadHandle(handle, EventArgsHandleType, &sec, (void **)&eventArgs)) != HandleError_None)
    {
        pluginContext->ThrowNativeError("Invalid EventArgs handle %x (error %d)", handle, handleError);
        return nullptr;
    }
    else return eventArgs;
}

cell_t smn_EventArgs_GetString(IPluginContext* pluginContext, const cell_t* params)
{
    Handle_t handle = static_cast<Handle_t>(params[1]);
    EventArgs* eventArgs = GetEventArgsFromHandle(handle, pluginContext);
    if (eventArgs != nullptr)
    {
        char *key;
        pluginContext->LocalToStringNULL(params[2], &key);

        //Get and write string value if it exists
        std::string* value = eventArgs->GetString(std::string(key));
        if (value != nullptr)
        {
            pluginContext->StringToLocalUTF8(params[3], params[4], value->c_str(), NULL);
            return 1;
        }
    }

    return 0;
}

cell_t smn_EventArgs_GetInt(SourcePawn::IPluginContext* pluginContext, const cell_t* params)
{
    Handle_t handle = static_cast<Handle_t>(params[1]);
    EventArgs* eventArgs = GetEventArgsFromHandle(handle, pluginContext);
    if (eventArgs != nullptr)
    {
        char *key;
        pluginContext->LocalToStringNULL(params[2], &key);

        int32* value = eventArgs->GetInt(std::string(key));
        if(value != nullptr) return *value;
    }
    
    return 0;
}

cell_t smn_EventArgs_GetFloat(SourcePawn::IPluginContext* pluginContext, const cell_t* params)
{
    Handle_t handle = static_cast<Handle_t>(params[1]);
    EventArgs* eventArgs = GetEventArgsFromHandle(handle, pluginContext);
    if (eventArgs != nullptr)
    {
        char *key;
        pluginContext->LocalToStringNULL(params[2], &key);

        float* value = eventArgs->GetFloat(std::string(key));
        if(value != nullptr) return sp_ftoc(*value);
    }
    
    return 0;
}

cell_t smn_EventArgs_GetBool(SourcePawn::IPluginContext* pluginContext, const cell_t* params)
{
    Handle_t handle = static_cast<Handle_t>(params[1]);
    EventArgs* eventArgs = GetEventArgsFromHandle(handle, pluginContext);
    if (eventArgs != nullptr)
    {
        char *key;
        pluginContext->LocalToStringNULL(params[2], &key);

        bool* value = eventArgs->GetBool(std::string(key));
        if(value != nullptr) return *value;
    }
    
    return 0;
}

cell_t smn_EventArgs_ContainsKey(SourcePawn::IPluginContext* pluginContext, const cell_t* params)
{
    Handle_t handle = static_cast<Handle_t>(params[1]);
    EventArgs* eventArgs = GetEventArgsFromHandle(handle, pluginContext);
    if (eventArgs != nullptr)
    {
        char *key;
        pluginContext->LocalToStringNULL(params[2], &key);

        return eventArgs->ContainsKey(std::string(key));
    }
    
    return 0;
}

cell_t smn_EventArgs_IsArray(SourcePawn::IPluginContext* pluginContext, const cell_t* params)
{
    Handle_t handle = static_cast<Handle_t>(params[1]);
    EventArgs* eventArgs = GetEventArgsFromHandle(handle, pluginContext);
    if (eventArgs != nullptr)
    {
        char *key;
        pluginContext->LocalToStringNULL(params[2], &key);

        Argument* arg = eventArgs->GetArgumentWithPath(std::string(key));
        if (arg != nullptr)
        {
            return arg->isArray;
        }
    }
    
    return 0;
}

cell_t smn_EventArgs_GetArrayLength(SourcePawn::IPluginContext* pluginContext, const cell_t* params)
{
    Handle_t handle = static_cast<Handle_t>(params[1]);
    EventArgs* eventArgs = GetEventArgsFromHandle(handle, pluginContext);
    if (eventArgs != nullptr)
    {
        char *key;
        pluginContext->LocalToStringNULL(params[2], &key);

        Argument* arg = eventArgs->GetArgumentWithPath(std::string(key));
        if (arg != nullptr)
        {
            return arg->length;
        }
    }
    
    return 0;
}
