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
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>

class CLuaCallback {
public:
  CLuaCallback(string n_HandlerName, luabind::object n_Callback) : m_HandlerName(n_HandlerName), m_Callback(n_Callback) {}
  bool MatchesHandlerName(string n_handlerName);
  template <class argtype> void Fire(argtype argument) {
    luabind::call_function<void>(m_Callback, argument);
  }
  
  void Fire() {
    luabind::call_function<void>(m_Callback);
  }
  
  template <class argtype1, class argtype2> void Fire(argtype1 argument1, argtype2 argument2) {
    luabind::call_function<void>(m_Callback, argument1, argument2);
  }

  
protected:
  string m_HandlerName;
  luabind::object m_Callback;
};

class CLuaScript {
protected:
  lua_State* m_Lua;
  vector<CLuaCallback *> m_Callbacks;
  string m_Filename;
public:
  CLuaScript(string n_Filename) : m_Filename(n_Filename) {}
  lua_State* getLua()  { return m_Lua; }
  
  
  bool Load();
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
public:
  CLuaScriptManager(CLuaContext* n_Context) : m_Context(n_Context) {}
  
  bool LoadScriptsFromDirectory(string dirName);
  bool LoadScript(string fileName);
  bool UnloadScript(string fileName);
  void Fire(CLuaEvent* event);
  
protected:
  vector <CLuaScript *> m_Scripts;
  CLuaContext* m_Context;
};
#endif
