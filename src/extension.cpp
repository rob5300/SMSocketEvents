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
#include "KeyValues.h"
#include "extension.h"
#include "TCPServer.hpp"

#define PORT 25570

using namespace SourceMod;
using namespace std;

SocketExtension g_socketExtension;
SMEXT_LINK(&g_socketExtension);

TCPServer* server;
map<string, IChangeableForward*> eventForwards;

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
    //TODO: Delete message and let keyvalues leak?
    EventMessage eventMessage;
    if (server->eventQueue.try_dequeue(eventMessage))
    {
        //We have a message to dispatch!
        if (eventForwards.find(eventMessage.name) != eventForwards.end())
        {
            auto forward = eventForwards[eventMessage.name];
            cell_t result = 0;
	        forward->PushCell(reinterpret_cast<cell_t>(eventMessage.args));
	        forward->Execute(&result);
 
	        if (result == Pl_Handled)
	        {
		        std::cout << "Event '" << eventMessage.name << "' sent" << std::endl;
	        }
        }
        else
        {
            std::cout << "Received Event '" << eventMessage.name << "' but there are 0 subscribers" << std::endl;
        }
    }
}

bool SocketExtension::SDK_OnLoad(char* error, size_t maxlength, bool late)
{
    smutils->AddGameFrameHook(OnGameFrame);

    server = new TCPServer(PORT);
    server->Start();
    Print(std::string("Started TCP server on ") + to_string(PORT));
    
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
    {"AddEventListener", AddEventListener},
    {"RemoveEventListener", RemoveEventListener},
    {"RemoveAllEventListeners", RemoveAllEventListeners},
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
