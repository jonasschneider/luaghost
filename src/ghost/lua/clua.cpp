#include "includes.h"
#include "ghost.h"
#include "game_base.h"
#include "gameplayer.h"
#include "game.h"
#include "ghost.h"
#include "util.h"
#include "config.h"
#include "language.h"
#include "socket.h"
#include "ghostdb.h"
#include "bnet.h"
#include "map.h"
#include "packed.h"
#include "savegame.h"
#include "replay.h"
#include "gameplayer.h"
#include "gameprotocol.h"


#include "boost/filesystem/operations.hpp"

using namespace std;


void CLuaContextGHost::ApplyToScript(CLuaScript* script) {
  using namespace luabind;
  
  module(script->getLua())
  [
    // Events
    class_<CLuaEvent>("CLuaEvent"),
    class_<CLuaGHostInitalizedEvent, CLuaEvent>("GHostInitalizedEvent")
      .def("GetGHost", &CLuaGHostInitalizedEvent::GetGHost),
      
    class_<CLuaGHostShuttingDownEvent, CLuaEvent>("CLuaGHostShuttingDownEvent")
      .def("GetGHost", &CLuaGHostShuttingDownEvent::GetGHost),
    
    class_<CLuaPlayerJoinedEvent, CLuaEvent>("CLuaPlayerJoinedEvent")
      .def("GetGame", &CLuaPlayerJoinedEvent::GetGame)
      .def("GetPlayer", &CLuaPlayerJoinedEvent::GetPlayer),
      
    class_<CLuaPlayersChatsIngameEvent, CLuaEvent>("CLuaPlayersChatsIngameEvent")
      .def("GetGame", &CLuaPlayersChatsIngameEvent::GetGame)
      .def("GetPlayer", &CLuaPlayersChatsIngameEvent::GetPlayer)
      .def("GetMessage", &CLuaPlayersChatsIngameEvent::GetMessage),

    // Domain classes
    class_<CGHost>("GHost")
      .def_readonly("version", &CGHost::m_Version),
    class_<CGamePlayer>("GamePlayer")
      .def("GetName", &CGamePlayer::GetName),
    class_<CBaseGame>("BaseGame")
      .def("SendAllChat", (void(CBaseGame::*)(string))&CBaseGame::SendAllChat)
  ];
}

// CLuaScript

bool CLuaScript :: Load() {
  using namespace luabind;
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

void CLuaScript :: Fire(CLuaEvent* event) {
  Log(string("[LUA] Firing ").append(event->GetLuaName()));
  using namespace luabind;
  for( vector<CLuaCallback *> :: iterator i = m_Callbacks.begin( ); i != m_Callbacks.end( ); i++ ) {
    if((*i)->MatchesHandlerName(event->GetLuaName())) {
      try {
        (*i)->Fire(event);
      } catch(error& e) {
        string msg = string("Lua Error while dispatching handler [") + event->GetLuaName() + string("]: ");
        Log(msg.append(lua_tostring(e.state(), -1)));
      }
    }
  }
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
      new_script->Call("init");
      m_Scripts.push_back(new_script);
    } else {
      cout << "[LUA] " << fileName << " has no 'init' function!";
      return false;
    }
    return true;
  } else
    return false;
}

bool CLuaScriptManager :: LoadScriptsFromDirectory(string dirName) {
  using namespace boost::filesystem;

  path full_path( initial_path<path>() );

  path dirpath = path(dirName);
  std::string filename;

  if ( !exists( dirpath ) || !is_directory( dirpath ) ) { 
    return false; }

  directory_iterator end_iter;
  for ( directory_iterator dir_itr( dirpath );
        dir_itr != end_iter;
        ++dir_itr ) {
    try {
      if( is_regular_file(dir_itr->status()) ) {
        filename = dir_itr->path().filename();
        if(filename.substr(filename.find(".lua")) == ".lua") // filename ends with .lua
          LoadScript(dir_itr->path().native_file_string());
      }
    } catch( const std::exception &ex ) { }
  }
  
  return true;
}

void CLuaScriptManager :: Fire(CLuaEvent* event) {
  cout << "[LUA] Firing " << event->GetLuaName() << endl;
  for( vector<CLuaScript *> :: iterator i = m_Scripts.begin( ); i != m_Scripts.end( ); i++ )
    (*i)->Fire(event);
}
  

bool CLuaScriptManager :: UnloadScript(string fileName) {
  
}
