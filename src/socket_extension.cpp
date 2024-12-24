/**
 * =============================================================================
 * SocketExtension
 * Robert Straub
 * =============================================================================
 */
#include <memory>
#include <string>
#include <filesystem>
#include <IPluginSys.h>
#include <ctime>
#include <iostream>
#include <sstream>
#include <map>
#include "socket_extension.h"
#include "TCPServer.hpp"
#include "Config.h"
#include "EventArgs.h"
#include "EventArgs_Handle.h"

using namespace SourceMod;
using namespace std;

SocketExtension g_socketExtension;
SMEXT_LINK(&g_socketExtension);

TCPServer* server = 0;
map<string, IChangeableForward*> eventForwards;
Config config;

IChangeableForward* GetOrAddForward(char* name)
{
    string nameString(name);
    if (eventForwards.find(nameString) != eventForwards.end())
    {
        return eventForwards[nameString];
    }
    else
    {
        IChangeableForward* newForward = forwards->CreateForwardEx(NULL, ET_Hook, 1, NULL, Param_Cell);
        eventForwards[nameString] = newForward;
        return newForward;
    }
}

/** 
 * Class to allow our convars to be properly registered.
 */
class BaseAccessor : public IConCommandBaseAccessor
{
public:
	bool RegisterConCommandBase(ConCommandBase *pCommandBase)
	{
		/* Always call META_REGCVAR instead of going through the engine. */
		return META_REGCVAR(pCommandBase);
	}
} s_BaseAccessor;

void SocketExtension::Print(string toPrint)
{
    std::ostringstream oss;
    oss << "[" << SMEXT_CONF_NAME << "] " << toPrint;
    std::cout << oss.str() << std::endl;
}

void OnGameFrame(bool simulated)
{
    //Try to dequeue and send many events this frame
    for (size_t i = 0; i < config.eventsPerFrame; i++)
    {
        EventMessage eventMessage;
        if (server->eventQueue.try_dequeue(eventMessage))
        {
            if (eventMessage.args == nullptr)
            {
                std::cout << "Event '" << eventMessage.name << "' was missing valid arguments." << std::endl;
                return;
            }

            //We have a message to dispatch!
            if (eventForwards.find(eventMessage.name) != eventForwards.end())
            {
                auto forward = eventForwards[eventMessage.name];
                cell_t result = 0;
            
                HandleError error = HandleError_None;
                const auto keyValuesHandle = handlesys->CreateHandle(EventArgsHandleType, eventMessage.args, NULL, myself->GetIdentity(), &error);
                if (error == 0)
                {
                    forward->Cancel();
                    forward->PushCell(keyValuesHandle);
	                forward->Execute(&result);
 
	                if (result == Pl_Handled)
	                {
		                std::cout << "Event '" << eventMessage.name << "' sent" << std::endl;
	                }
                }
                else
                {
                    std::cout << "Handle creation error " << error << std::endl;
                }
            }
            else
            {
                std::cout << "Received Event '" << eventMessage.name << "' but there are 0 subscribers" << std::endl;
            }
        }
        //Stop now as no events can be de queued.
        else break;
    }
}

bool SocketExtension::SDK_OnLoad(char* error, size_t maxlength, bool late)
{
    try
    {
#if _WIN32
        string configPath = string(g_SMAPI->GetBaseDir()) + "\\cfg\\sourcemod\\socketevents.ini";
#else
        string configPath = string(g_SMAPI->GetBaseDir()) + "/cfg/sourcemod/socketevents.ini";
#endif
        const bool parsedConfig = textparsers->ParseFile_INI(configPath.c_str(), &config, NULL, NULL);
    }
    catch (std::exception e)
    {
        std::cout << "Exception while loading config: " << e.what() << std::endl;
        return false;
    }

    EventArgsHandleType = handlesys->CreateType("EventArgs", &eventArgsHandler, 0, NULL, NULL, myself->GetIdentity(), NULL);
    smutils->AddGameFrameHook(OnGameFrame);

    server = new TCPServer(config.port);
    server->Start();
    Print(std::string("Started TCP server on ") + to_string(config.port));
    
    return true;
}

void SocketExtension::SDK_OnUnload()
{
    //Unload all forwards
    for (auto forwardPair : eventForwards)
    {
        forwards->ReleaseForward(forwardPair.second);
    }
    eventForwards.clear();
    
    smutils->RemoveGameFrameHook(OnGameFrame);

    if (server != nullptr)
    {
        server->Stop();
        delete(server);
    }
}

static cell_t AddEventListener(IPluginContext *pContext, const cell_t *params)
{
    char *str;
	pContext->LocalToString(params[1], &str);
    const auto forward = GetOrAddForward(str);
	forward->AddFunction(pContext, static_cast<funcid_t>(params[2]));
	return 1;
}
 
static cell_t RemoveEventListener(IPluginContext *pContext, const cell_t *params)
{
    char *str;
	pContext->LocalToString(params[1], &str);
    const auto forward = GetOrAddForward(str);
	IPluginFunction *pFunction = pContext->GetFunctionById(static_cast<funcid_t>(params[2]));
	forward->RemoveFunction(pFunction);
	return 1;
}

static cell_t RemoveAllEventListeners(IPluginContext *pContext, const cell_t *params)
{
    char *str;
	pContext->LocalToString(params[1], &str);
    const auto forward = GetOrAddForward(str);
	IPluginFunction *pFunction = pContext->GetFunctionById(static_cast<funcid_t>(params[2]));
	//forward->RemoveFunctionsOfPlugin(pContext->GetIdentity());
	return 0;
}

const sp_nativeinfo_t NativeFunctions [] =
{
    {"AddEventListener",            AddEventListener},
    {"RemoveEventListener",         RemoveEventListener},
    {"RemoveAllEventListeners",     RemoveAllEventListeners},
    {"EventArgs_GetString",         smn_EventArgs_GetString},
    {"EventArgs_GetInt",            smn_EventArgs_GetInt},
    {"EventArgs_GetFloat",          smn_EventArgs_GetFloat},
    {"EventArgs_GetBool",           smn_EventArgs_GetBool},
    {"EventArgs_ContainsKey",       smn_EventArgs_ContainsKey},
    {"EventArgs_IsArray",           smn_EventArgs_IsArray},

    {"EventArgs.GetString",         smn_EventArgs_GetString},
    {"EventArgs.GetInt",            smn_EventArgs_GetInt},
    {"EventArgs.GetFloat",          smn_EventArgs_GetFloat},
    {"EventArgs.GetBool",           smn_EventArgs_GetBool},
    {"EventArgs.ContainsKey",       smn_EventArgs_ContainsKey},
    {"EventArgs.IsArray",           smn_EventArgs_IsArray},
    {NULL, NULL},
};

void SocketExtension::SDK_OnAllLoaded ()
{
    sharesys->AddNatives (myself, NativeFunctions);
}

bool SocketExtension::SDK_OnMetamodLoad(ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    //GET_V_IFACE_ANY(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
    //ConVar_Register(0, &s_BaseAccessor);
    return true;
}
