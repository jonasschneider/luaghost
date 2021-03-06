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

#include "lua/clua.h"
#include "lua/rc.h"

using namespace std;

// CLuaContextGHost

void CLuaContextGHost :: ApplyToScript(CLuaScript* script) {
  using namespace luabind;
  
  module(script->getLua())
  [
    // Events
    class_<CLuaEvent>("CLuaEvent"),
    class_<CLuaGHostInitalizedEvent, CLuaEvent>("GHostInitalizedEvent")
      .def("GetGHost", &CLuaGHostInitalizedEvent::GetGHost),
      
    class_<CLuaGHostShuttingDownEvent, CLuaEvent>("GHostShuttingDownEvent")
      .def("GetGHost", &CLuaGHostShuttingDownEvent::GetGHost),
    
    class_<CLuaPlayerJoinedEvent, CLuaEvent>("PlayerJoinedEvent")
      .def("GetGame", &CLuaPlayerJoinedEvent::GetGame)
      .def("GetPlayer", &CLuaPlayerJoinedEvent::GetPlayer),
      
    class_<CLuaGamePlayerChatEvent, CLuaEvent>("GamePlayerChatEvent")
      .def("GetGame", &CLuaGamePlayerChatEvent::GetGame)
      .def("GetPlayer", &CLuaGamePlayerChatEvent::GetPlayer)
      .def("GetMessage", &CLuaGamePlayerChatEvent::GetMessage),
   
    class_<CLuaRCCommandReceivedEvent, CLuaEvent>("RCCommandReceivedEvent")
      .def("GetGHost", &CLuaRCCommandReceivedEvent::GetGHost)
      .def("GetCommand", &CLuaRCCommandReceivedEvent::GetCommand)
      .def("GetArg", &CLuaRCCommandReceivedEvent::GetArg)
      .def("GetArgCount", &CLuaRCCommandReceivedEvent::GetArgCount),
      /*.def("GetClient", &CLuaRCCommandReceivedEvent::GetClient),*/
    
    // Domain classes
    class_<CGHost>("GHost")
      .def_readonly("version", &CGHost::m_Version)
      .def_readonly("current_game", &CGHost::m_CurrentGame)
      .def("LoadMap", &CGHost::LoadMap)
      .def("CreateGame", (void(CGHost::*)(CMap*, bool, string, string))&CGHost::CreateGame),
    
    class_<CGamePlayer>("GamePlayer")
      .def("GetName", &CGamePlayer::GetName),
    
    class_<CBaseGame>("BaseGame")
      .def("SendAllChat", (void(CBaseGame::*)(string))&CBaseGame::SendAllChat)
      .def("Unhost", &CBaseGame::Unhost)
      
      .def("GetTeamOfPlayer", &CBaseGame::GetTeamOfPlayer)
      .def("GetHostCounter", &CBaseGame::GetHostCounter)
      .def("GetNumPlayersInTeam", &CBaseGame::GetNumPlayersInTeam)
      .def("GetCountdownStarted", &CBaseGame::GetCountDownStarted),
    
    class_<CMap>("Map")
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
    
    m_Context->ApplyToScript(this);
    if(HasFunction("init")) {
      Call("init");
      return true;
    } else {
      Log("No init function defined!");
    }
  }
  return false;
}

CLuaScript :: ~CLuaScript() {
  Unload();
}

void CLuaScript :: Unload() {
  m_Callbacks.clear();
  lua_close(m_Lua);
  Log("Unloaded.");
}

bool CLuaScript :: Reload() {
  Log("Reloading ...");
  Unload();
  return Load();
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


bool CLuaScript :: HasFunction(const char *name) {
  using namespace luabind;
  
  object func = globals(m_Lua)[name];
  
  return func && type(func) == LUA_TFUNCTION;
}


void CLuaScript :: Fire(CLuaEvent* event) {
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


void CLuaScript :: Log(string msg) {
  std::cout << "[" << m_Filename << "] " << msg << std::endl;
}


void CLuaScript :: Lua_Register(string handlerName, const luabind::object &callback) {
  m_Callbacks.push_back(new CLuaCallback(handlerName, callback));
}


void CLuaScript :: Lua_Unregister(const luabind::object &callback) {
  
}


// CLuaScriptManager

bool CLuaScriptManager :: LoadScript(string fileName) {
  CLuaScript* new_script = new CLuaScript(fileName, m_Context);
  if(new_script->Load()) {
    m_Scripts.push_back(new_script);
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

void CLuaScriptManager :: ReloadScripts() {
  for( vector<CLuaScript *> :: iterator i = m_Scripts.begin( ); i != m_Scripts.end( ); i++ )
    (*i)->Reload();
}

void CLuaScriptManager :: Fire(CLuaEvent* event, bool silent) {
  if(!silent)
    cout << "[LUA] Firing " << event->GetLuaName() << endl;
  for( vector<CLuaScript *> :: iterator i = m_Scripts.begin( ); i != m_Scripts.end( ); i++ )
    (*i)->Fire(event);
}

CLuaScript* CLuaScriptManager :: GetScriptByFilename(string Filename) {
  for( vector<CLuaScript *> :: iterator i = m_Scripts.begin( ); i != m_Scripts.end( ); i++ ) {
    if((*i)->GetFilename() == Filename) {
      return (*i);
    }
  }
}

bool CLuaScriptManager :: UnloadScript(string Filename) {
  for( vector<CLuaScript *> :: iterator i = m_Scripts.begin( ); i != m_Scripts.end( ); i++ ) {
    if((*i)->GetFilename() == Filename) {
      m_Scripts.erase(i);
      return true;
    }
  }
  cout << "[LUA] Could not unload; " << Filename << " is not loaded." << endl;
  return false;
}
