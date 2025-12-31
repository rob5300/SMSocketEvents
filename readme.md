# Sockets Extension

A sourcemod native extension.

Invoke custom events on your soucemod server with custom data via websockets (or raw TCP) and listen to them from source pawn. 
Message bodies are JSON.

Uses boost beast, boost asio and openssl.

# How to use
## Websockets
Connect your websocket client to the configured port (443 or 80).
## Message format
Events are sent as JSON. The event name is required as ``event`` alongwith any arguments as an object ``args``. Example:
```
{
    "event": "myEventName",
    "args": {
        
    },
    "token": "mytoken123"
}
```
If enabled you must also provide a valid token as ``token`` (these are user configured in the extension config).

# Recieve event
Use ``AddEventListener()`` to subscribe to an event with a specific name. The EventArgs object given contains the data from the event payload similarly to KeyValues.

The event args object has many functions to get your data as a string, int, bool and float. Arrays are also supported. If you have objects within objects then these can be retrieved as EventArg objects. Similarly to KeyValues, you can specify a path as a key to get values quickly from nested objects.

EventArgs objects must be disposed of when no longer used, these are Handles.

Example:
```
public void OnPluginStart()
{
    AddEventListener("my_event", EventCallback1);
}

void EventCallback1(EventArgs args)
{
    char value[256];
    if(args.GetString("string", value, 256))
    {
        LogAction(0, -1, "string: %s", value);
    }

    if(args.GetString("player/name", value, 256))
    {
        LogAction(0, -1, "Player name: %s", value);
    }

    int num = args.GetInt("int");
    IntToString(num, value, 256);
    LogAction(0, -1, "int: %i", num);

    delete args;
}
```

## TCP
Check [tcp_readme.md](./tcp_readme.md) for info on how to use raw TCP mode.

## Config
Configuration is read from ``sourcemod/configs/socketevents.json`` and is JSON. Below is a sample of the config:
```
{
    "port": 443,  //Port to bind and listen to
    "websockets": true,  //If websockets should be used
    "eventsPerFrame": 4,  //How many events max per game tick to send to listeners
    "tokens": [
        "mytoken123"
    ]
}
```

## Building
Ensure you have the required dependencies for [building Sourcemod](https://wiki.alliedmods.net/Building_sourcemod). Ambuild is not used by this project.

Metamod, Sourcemod and hl2sdk-tf2 are needed. These need to be in the same directory as this project (../sourcemod, ../hl2sdk-tf2, ../mmsource-1.10)
### Linux:
Install/update ``g++ gcc g++-multilib gcc-multilib libboost-all-dev openssl:i386`` via your package manager.

Use the provided make file via ``make``. Requires GCC 8+ (Ambuild cannot be used).
### Windows:
Install vcpkg and install from manifest: ``vcpkg install --triplet x86-windows``. This will install boost asio and openssl

Open and build using the solution file (``socket_ext.sln``) in VS 2022+
