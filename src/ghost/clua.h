using namespace std;

extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

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
  
  bool HasFunction(const char* name);
  
  void Lua_Register(string handlerName, const luabind::object &callback);
  void Lua_Unregister(const luabind::object &callback);
  void Log(string msg); // only to be used from inside and Lua!
  
  template <class argtype> void Fire(string handlerName, argtype argument) {
    Log(string("[LUA] Firing ").append(handlerName));
    using namespace luabind;
    for( vector<CLuaCallback *> :: iterator i = m_Callbacks.begin( ); i != m_Callbacks.end( ); i++ ) {
      if((*i)->MatchesHandlerName(handlerName)) {
        try {
          (*i)->Fire(argument);
        } catch(error& e) {
          string msg = string("Lua Error while dispatching handler [") + handlerName + string("]: ");
          Log(msg.append(lua_tostring(e.state(), -1)));
        }
      }
    }
  }
  
  void Fire(string handlerName) {
    Log(string("[LUA] Firing ").append(handlerName));
    using namespace luabind;
    for( vector<CLuaCallback *> :: iterator i = m_Callbacks.begin( ); i != m_Callbacks.end( ); i++ ) {
      if((*i)->MatchesHandlerName(handlerName)) {
        try {
          (*i)->Fire();
        } catch(error& e) {
          string msg = string("Lua Error while dispatching handler [") + handlerName + string("]: ");
          Log(msg.append(lua_tostring(e.state(), -1)));
        }
      }
    }
  }
  
  template <class argtype> void Call(const luabind::object &method, argtype argument) {
    using namespace luabind;
    
    try {
      call_function<void>(method, argument);
    } catch(error& e) {
      Log(string("Lua Error: ").append(lua_tostring(e.state(), -1)));
    }
  }
  
  template <class argtype> void Call(const char* method, argtype argument) {
    using namespace luabind;
    
    try {
      call_function<void>(m_Lua, method, argument);
    } catch(error& e) {
      Log(string("Lua Error: ").append(lua_tostring(e.state(), -1)));
    }
  }
  
  template<class argtype1, class argtype2> void Fire(string handlerName, argtype1 argument1, argtype2 argument2) {
    Log(string("[LUUA] Firing ").append(handlerName));
    using namespace luabind;
    for( vector<CLuaCallback *> :: iterator i = m_Callbacks.begin( ); i != m_Callbacks.end( ); i++ ) {
      if((*i)->MatchesHandlerName(handlerName)) {
        try {
          (*i)->Fire(argument1, argument2);
        } catch(error& e) {
          string msg = string("Lua Error while dispatching handler [") + handlerName + string("]: ");
          Log(msg.append(lua_tostring(e.state(), -1)));
        }
      }
    }
  }
  
  template<class argtype1, class argtype2> void Call(const luabind::object &method, argtype1 argument1, argtype2 argument2) {
    using namespace luabind;
    
    try {
      call_function<void>(method, argument1, argument2);
    } catch(error& e) {
      Log(string("Lua Error: ").append(lua_tostring(e.state(), -1)));
    }
  }
  
  template<class argtype1, class argtype2> void Call(const char* method, argtype1 argument1, argtype2 argument2) {
    using namespace luabind;
    
    try {
      call_function<void>(m_Lua, method, argument1, argument2);
    } catch(error& e) {
      Log(string("Lua Error: ").append(lua_tostring(e.state(), -1)));
    }
  }

  
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
  
  void Fire(string handlerName) {
    cout << "[LUA] Firing " << handlerName << endl;
    for( vector<CLuaScript *> :: iterator i = m_Scripts.begin( ); i != m_Scripts.end( ); i++ )
      (*i)->Fire(handlerName);
  }
  
  template<class argtype> void Fire(string handlerName, argtype argument) {
    cout << "[LUA] Firing " << handlerName << endl;
    for( vector<CLuaScript *> :: iterator i = m_Scripts.begin( ); i != m_Scripts.end( ); i++ )
      (*i)->Fire(handlerName, argument);
  }
  
  template<class argtype1, class argtype2> void Fire(string handlerName, argtype1 argument1, argtype2 argument2) {
    cout << "[LUA] Firing " << handlerName << endl;
    for( vector<CLuaScript *> :: iterator i = m_Scripts.begin( ); i != m_Scripts.end( ); i++ )
      (*i)->Fire(handlerName, argument1, argument2);
  }
  
  
protected:
  vector <CLuaScript *> m_Scripts;
  CLuaContext* m_Context;
};
