#ifndef CLUA_EVENTS_H
#define CLUA_H


class CGHost;
class CBaseGame;
class CGamePlayer;

class CLuaEvent {
public:
  virtual std::string GetLuaName() {}
  
};

/* Called when GHost is initialized */
class CLuaGHostInitalizedEvent : public CLuaEvent {
protected:
  CGHost* m_GHost;
public:
  CLuaGHostInitalizedEvent(CGHost* n_GHost) : m_GHost(n_GHost) {}
  std::string GetLuaName() { return "GHostInitalized"; }
  CGHost* GetGHost() { return m_GHost; }
};


/* Called when GHost is shutting down */
class CLuaGHostShuttingDownEvent : public CLuaEvent {
protected:
  CGHost* m_GHost;
public:
  CLuaGHostShuttingDownEvent(CGHost* n_GHost) : m_GHost(n_GHost) {}
  std::string GetLuaName() { return "GHostShuttingDown"; }
  CGHost* GetGHost() { return m_GHost; }
};


/* Called when a player has successfully joined a game (includes admin game) */
class CLuaPlayerJoinedEvent : public CLuaEvent {
protected:
  CBaseGame* m_Game;
  CGamePlayer* m_Player;
public:
  CLuaPlayerJoinedEvent(CBaseGame* n_Game, CGamePlayer* n_Player) : m_Game(n_Game), m_Player(n_Player) {}
  std::string GetLuaName() { return "PlayerJoined"; }
  CBaseGame* GetGame() { return m_Game; }
  CGamePlayer* GetPlayer() { return m_Player; }
};


/* Called when a player chats ingame or in a game lobby */
class CLuaGamePlayerChatEvent : public CLuaEvent {
protected:
  CBaseGame* m_Game;
  CGamePlayer* m_Player;
  std::string m_Message;
  bool m_Ingame;
public:
  CLuaGamePlayerChatEvent(CBaseGame* n_Game, CGamePlayer* n_Player, std::string n_Message, bool n_Ingame) : m_Game(n_Game), m_Player(n_Player), m_Message(n_Message), m_Ingame(n_Ingame) {}
  std::string GetLuaName() { return "GamePlayerChat"; }
  CBaseGame* GetGame() { return m_Game; }
  CGamePlayer* GetPlayer() { return m_Player; }
  std::string GetMessage() { return m_Message; }
  bool GetIngame() { return m_Ingame; }
};
#endif
