# Sockets Extension

A sourcemod native extension.

Send custom events with custom data to a TCP socket and listen to them from source pawn. 

# How to use
## Send event payload
Currently, an event payload must be sent in 2 parts, the event header and then the event payload. Hopefully in future this can be simplified for the header + body to be sent together.
### Header
The header is 400 bytes in length:
- ``EVENTMSG`` string (char[8])
- Payload byte length (int64)
- Sha256 payload signature (char[384])

If payload signature verification (secure mode) is not enabled then the signature is not checked (so can be all 0's).
### Body
The body should be a utf8 json string with the layout below. Its byte length (in utf8) must match what was specified in the header sent previously.
```
{
    "event": "myEventName",
    "args": {} //Your event arguments
}
```

Currently no response is given by the server when data is sent. Check the server console for log messages reporting if the message was recieved correctly or not.

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

## Config
Configuration is read from ``sourcemod/configs/socketevents.ini``. Below is a sample of the config:
```
[server]
port = 25570        //Port to bind to and listen to (TCP)
public_key_file = test_key.pem      //public key in PEM format, if secure mode is enabled
secure = true       //If the payload signature should be verified using the configured public key
eventsPerFrame = 4      //How many events max per game tick to send to listeners
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
