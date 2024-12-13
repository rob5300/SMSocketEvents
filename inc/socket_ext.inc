typedef EventCallback = function Action (KeyValues keyValues);
 
native void AddEventListener(const char[] name, EventCallback listener);
native void RemoveEventListener(const char[] name, EventCallback listener);
native void RemoveAllEventListeners(const char[] name);