#include "lua/clua.h"

#define RC_HEADER_CONSTANT 209

#define RC_REQUEST_STATUS 210

#define RC_RESPONSE_AVAILABLE 230

class CLuaRCRequest {
protected:
  int m_Command;
  BYTEARRAY m_Packet;
public:
  CLuaRCRequest(int n_Command, BYTEARRAY n_Packet) : m_Command(n_Command), m_Packet(n_Packet) {}
  int GetCommand() { return m_Command; }
};

class CLuaRCClientHandler {
protected:
  CGHost* m_GHost;
  CTCPSocket* m_Socket;
  std::queue<CLuaRCRequest *> m_Requests;

public:
  CLuaRCClientHandler(CTCPSocket* n_Socket, CGHost* n_GHost) : m_Socket(n_Socket), m_GHost(n_GHost) {}
  CTCPSocket *GetSocket() { return m_Socket; }
  
  bool SetFD( void *fd, void *send_fd, int *nfds );
  bool Update( void *fd, void *send_fd );
};

class CLuaRC {
protected:
  CGHost* m_GHost;
  CTCPServer* m_Socket;
  vector<CLuaRCClientHandler *> m_Handlers;

public:
  CLuaRC(CGHost* n_GHost);
  
  void Update( void *fd, void *send_fd );
  unsigned int SetFD( void *fd, void *send_fd, int *nfds );
  CTCPSocket* GetSocket() { return m_Socket; }
};
