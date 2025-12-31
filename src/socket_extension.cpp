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
#include <fstream>
#include "socket_extension.h"
#include "server/WebsocketServer.h"
#include "server/TCPServer.hpp"
#include "Config.h"
#include "EventArgs.h"
#include "EventArgs_Handle.h"
#include "../ext/nlohmann/json.hpp"

using namespace SourceMod;
using namespace std;
using namespace std::filesystem;

SocketExtension g_socketExtension;
SMEXT_LINK(&g_socketExtension);

std::unique_ptr<Server> server;
map<string, IChangeableForward*> eventForwards;
Config config;
const std::string ConfigName = "socketevents.json";

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

void SocketExtension::PrintError(std::string toPrint)
{
    std::ostringstream oss;
    oss << "[" << SMEXT_CONF_NAME << "] ERROR: " << toPrint;
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
                SocketExtension::PrintError("Event '" + eventMessage.name + "' was missing valid arguments.");
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
                        SocketExtension::Print("Event '" + eventMessage.name + "' sent");
	                }
                }
                else
                {
                    SocketExtension::PrintError("EventArgs Handle creation error: " + std::to_string(error));
                    delete eventMessage.args;
                }
            }
            else
            {
                SocketExtension::PrintError("Received Event '" + eventMessage.name + "' but there are 0 subscribers");
                //Delete args as it would otherwise leak
                delete eventMessage.args;
            }
        }
        //Stop now as no events can be de queued.
        else break;
    }
}

bool SocketExtension::SDK_OnLoad(char* error, size_t maxlength, bool late)
{
    const path configFolderPath = path(g_SMAPI->GetBaseDir()) / path("addons/sourcemod/configs");
    const path configFilePath = configFolderPath / path(ConfigName);

    try
    {
        if (filesystem::exists(configFilePath))
        {
            try
            {
                const std::ifstream configStream(configFilePath);
                std::stringstream configStringStream;
                configStringStream << configStream.rdbuf();
                config = nlohmann::json::parse(configStringStream).get<Config>();
            }
            catch (const exception& exception)
            {
                Print(exception.what());
                Print("Failed to parse config, defaults will be used.");
            }

            if (config.secure && !config.websockets)
            {
                const bool keyLoaded = config.LoadPEMPublicKey(configFolderPath);
                if (!keyLoaded)
                {
                    Print("Failed to load public key file '" + config.public_key_file +"'. A public key in PEM format is required for secure mode in tcp mode!");
                }
            }
        }
        else Print("Config '" + ConfigName + "' not present, defaults will be used.");
    }
    catch (std::exception e)
    {
        std::cout << "Exception while loading config: " << e.what() << std::endl;
        return false;
    }

    EventArgsHandleType = handlesys->CreateType("EventArgs", &eventArgsHandler, 0, NULL, NULL, myself->GetIdentity(), NULL);
    smutils->AddGameFrameHook(OnGameFrame);

    //Make web socket server or tcp server
    if(config.websockets)
    {
        Print("Will use web socket server");
        server = std::make_unique<WebsocketServer>(&config);
    }
    else
    {
        Print("Will use TCP server");
        server = std::make_unique<TCPServer>(&config);
    }

    server->Start();
    Print(std::string("Started event server on port: ") + to_string(config.port));
    
    if (!config.websockets)
    {
        //Print message indicating message signature verification state
        const bool isSecure = config.secure && !config.public_key_pem.empty();
        Print(isSecure ? std::string("Message signature verification is enabled.") : std::string("Message signature verification is disabled (Enable in config)."));
    }
    
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

    if (!server)
    {
        server->Stop();
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
    {"EventArgs_GetArrayLength",    smn_EventArgs_GetArrayLength},

    {"EventArgs.GetString",         smn_EventArgs_GetString},
    {"EventArgs.GetInt",            smn_EventArgs_GetInt},
    {"EventArgs.GetFloat",          smn_EventArgs_GetFloat},
    {"EventArgs.GetBool",           smn_EventArgs_GetBool},
    {"EventArgs.ContainsKey",       smn_EventArgs_ContainsKey},
    {"EventArgs.IsArray",           smn_EventArgs_IsArray},
    {"EventArgs.GetArrayLength",    smn_EventArgs_GetArrayLength},
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
