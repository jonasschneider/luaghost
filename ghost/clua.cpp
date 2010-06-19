#include "ghost.h"
using namespace std;


void CLuaContextGHost::ApplyToScript(CLuaScript* script) {
  using namespace luabind;
  
  module(script->getLua())
  [
    class_<CGHost>("GHost")
      .def_readonly("version", &CGHost::m_Version)
  ];
}

// CLuaScript

bool CLuaScript :: Load() {
  using namespace luabind;
  Log("Loading...");
  m_Lua = lua_open();
	luaL_openlibs(m_Lua);
  luabind::open(m_Lua);
  
  int retval = luaL_dofile(m_Lua, m_Filename.c_str());
  if(retval == LUA_ERRSYNTAX)
    Log("Syntax error while loading!");
  else if(retval == LUA_ERRMEM)
    Log("Memory allocation error while loading!");
  else {
    Log("Loaded.");
    module(m_Lua)
    [
      class_<CLuaScript>("CLuaScript")
        .def("Register", &CLuaScript::Lua_Register)
        .def("Unregister", &CLuaScript::Lua_Unregister)
        .def("Log", &CLuaScript::Log)
    ];
    luabind::globals(m_Lua)["Controller"] = this;
  }
}


void CLuaScript :: Call(const char* method) {
  using namespace luabind;
  
  try {
    call_function<void>(m_Lua, method);
  } catch(error& e) {
    Log(string("Lua Error: ").append(lua_tostring(e.state(), -1)));
  }
}





void CLuaScript :: Call(const luabind::object &method) {
  using namespace luabind;
  
  try {
    call_function<void>(method);
  } catch(error& e) {
    Log(string("Lua Error: ").append(lua_tostring(e.state(), -1)));
  }
}




void CLuaScript :: Log(string msg) {
  std::cout << "[" << m_Filename << "] " << msg << std::endl;
}

void CLuaScript :: Lua_Register(string handlerName, const luabind::object &callback) {
  m_Callbacks.push_back(new CLuaCallback(handlerName, callback));
}

void CLuaScript :: Lua_Unregister(const luabind::object &callback) {
  
}

bool CLuaScript :: HasFunction(const char *name) {
  using namespace luabind;
  
  object g = globals(m_Lua);
  object func = g[name];
  
  if( func )
  {
          if( type(func) == LUA_TFUNCTION )
                  return true;
  }
  
  return false;
}




// CLuaCallback

bool CLuaCallback :: MatchesHandlerName(string n_HandlerName) {
  return m_HandlerName == n_HandlerName;
}


bool CLuaScriptManager :: LoadScript(string fileName) {
  CLuaScript* new_script = new CLuaScript(fileName);
  if(new_script->Load()) {
    m_Context->ApplyToScript(new_script);
    if(new_script->HasFunction("init")) {
      new_script->Call("init", new_script);
      m_Scripts.push_back(new_script);
    } else {
      cout << "[LUA] " << fileName << " has no 'init' function!";
      return false;
    }
    return true;
  } else
    return false;
}

bool CLuaScriptManager :: UnloadScript(string fileName) {
  
}
