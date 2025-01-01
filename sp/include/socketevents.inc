typedef EventCallback = function void (EventArgs args);
 
native void AddEventListener(const char[] name, EventCallback listener);
native void RemoveEventListener(const char[] name, EventCallback listener);
native void RemoveAllEventListeners(const char[] name);

methodmap EventArgs < Handle
{
    public native bool GetString(const char[] key, char[] value, int maxlength);
    public native int GetInt(const char[] key);
    public native float GetFloat(const char[] key);
    public native bool GetBool(const char[] key);
    public native bool ContainsKey(const char[] key);
    public native bool IsArray(const char[] key);
    public native int GetArrayLength(const char[] key);
}

//Fallback functions using Handle
native bool EventArgs_GetString(Handle eventArgs, const char[] key, char[] value, int maxlength);
native int EventArgs_GetInt(Handle eventArgs, const char[] key);
native float EventArgs_GetFloat(Handle eventArgs, const char[] key);
native bool EventArgs_GetBool(Handle eventArgs, const char[] key);
native bool EventArgs_ContainsKey(Handle eventArgs, const char[] key);
native bool EventArgs_IsArray(Handle eventArgs, const char[] key);
native int EventArgs_GetArrayLength(Handle eventArgs, const char[] key);