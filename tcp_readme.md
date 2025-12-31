## TCP
### Send event payload
Currently, an event payload must be sent in 2 parts, the event header and then the event payload. Hopefully in future this can be simplified for the header + body to be sent together.
#### Header
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

## Config
There are some configuration options used by TCP mode:
- ``public_key_file``  public key in PEM format, if secure mode is enabled
- ``secure``  If the payload signature should be verified using the configured public key
