#ifndef CLUA_H
#define CLUA_H
using namespace std;

extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}


#include "lua/clua_events.h"
#include <luabind/luabind.hpp>

class CLuaCallback {
protected:
  string m_HandlerName;
  luabind::object m_Callback;

public:
  CLuaCallback(string n_HandlerName, luabind::object n_Callback) : m_HandlerName(n_HandlerName), m_Callback(n_Callback) {}
  bool MatchesHandlerName(string n_HandlerName) { return m_HandlerName == n_HandlerName; }

  void Fire(CLuaEvent* event) { luabind::call_function<void>(m_Callback, event); }
};

class CLuaContext;

class CLuaScript {
protected:
  lua_State* m_Lua;
  vector<CLuaCallback *> m_Callbacks;
  string m_Filename;
  CLuaContext* m_Context;
  
public:
  CLuaScript(string n_Filename, CLuaContext* n_Context) : m_Filename(n_Filename), m_Context(n_Context) {}
  ~CLuaScript();
  lua_State* getLua()  { return m_Lua; }
  string GetFilename() { return m_Filename; }
  
  bool Load();
  void Unload();
  bool Reload();
  void Call(const char* method);
  void Call(const luabind::object &method);
  void Fire(CLuaEvent* event);
  bool HasFunction(const char* name);
  
  // Lua-accessible functions
  void Lua_Register(string handlerName, const luabind::object &callback);
  void Lua_Unregister(const luabind::object &callback);
  void Log(string msg); // only to be used from inside and Lua!
};


class CLuaContext {
public: virtual void ApplyToScript(CLuaScript* script) {}
};

class CLuaContextGHost : public CLuaContext {
public: void ApplyToScript(CLuaScript* script);
};


class CLuaScriptManager {
protected:
  vector <CLuaScript *> m_Scripts;
  CLuaContext* m_Context;
  
  CLuaScript* GetScriptByFilename(string Filename);

public:
  CLuaScriptManager(CLuaContext* n_Context) : m_Context(n_Context) {}
  
  bool LoadScriptsFromDirectory(string dirName);
  bool LoadScript(string fileName);
  bool UnloadScript(string fileName);
  void Fire(CLuaEvent* event, bool silent = false);
  void ReloadScripts();
};
#endif
