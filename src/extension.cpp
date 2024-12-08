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
#include "extension.h"
#include "tier1/iconvar.h"
#include "convar.h"
#include "icvar.h"
#include <ctime>
#include <iostream>
#include "TCPServer.hpp"

#define PORT 25570

using namespace SourceMod;
using namespace std;

SocketExtension g_Sample;
SMEXT_LINK(&g_Sample);

TCPServer* server;

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
    std::cout << std::format("[{}] {}.\n", SMEXT_CONF_NAME, toPrint) << std::endl;
}

bool SocketExtension::SDK_OnLoad(char* error, size_t maxlength, bool late)
{
    server = new TCPServer(PORT);
    server->Start();
    Print(std::format("Started TCP server on {}", PORT));

    return true;
}

void SocketExtension::SDK_OnUnload()
{
    if (server != nullptr)
    {
        server->Stop();
        delete(server);
    }
}

const sp_nativeinfo_t NativeFunctions [] = {
    {NULL, NULL},
};

void SocketExtension::SDK_OnAllLoaded ()
{
    //sharesys->AddNatives (myself, NativeFunctions);
}

bool SocketExtension::SDK_OnMetamodLoad(ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    GET_V_IFACE_ANY(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
    ConVar_Register(0, &s_BaseAccessor);
    return true;
}
