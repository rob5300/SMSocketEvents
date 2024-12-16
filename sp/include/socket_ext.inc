typedef EventCallback = function void (Handle args);
 
native void AddEventListener(const char[] name, EventCallback listener);
native void RemoveEventListener(const char[] name, EventCallback listener);
native void RemoveAllEventListeners(const char[] name);

//methodmap EventArgs < Handle
//{
//    public native bool GetString(const char[] key, char[] value, int maxlength);
//}

native bool EventArgs_GetString(Handle kv, const char[] key, char[] value, int maxlength);