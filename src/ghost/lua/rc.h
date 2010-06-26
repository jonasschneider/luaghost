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

class CLuaRCHandler {
protected:
  CGHost* m_GHost;
  CTCPSocket* m_Socket;
  std::queue<CLuaRCRequest *> m_Requests;

public:
  CLuaRCHandler(CTCPSocket* n_Socket, CGHost* n_GHost);
  CTCPSocket *GetSocket() { return m_Socket; }
  bool PrintInfo( void *fd, void *send_fd, int *nfds ) {
    //std::cout << "My socket is at " << m_Socket << std::endl;
    if(m_Socket) {
      //std::cout << "Hello from handler" << std::endl;
      if(m_Socket->HasError()) {
        //std::cout << "My socket has an error." << std::endl;
        //std::cout << "The error is" << m_Socket->GetErrorString() << std::endl;
      } else {
        //std::cout << "My socket is fine." << std::endl;
        m_Socket->SetFD( (fd_set *)fd, (fd_set *)send_fd, nfds );
        return true;
      }
    } else {
      //std::cout << "My socket is NULL!" << std::endl;
    }
    return false;
   }
  
  bool Update( void *fd, void *send_fd );
};

class CLuaRC {
protected:
  CGHost* m_GHost;
  CTCPServer* m_Socket;
  vector<CLuaRCHandler *> m_Handlers;

public:
  CLuaRC(CGHost* n_GHost);
  
  void Update( void *fd, void *send_fd );
  unsigned int SetFD( void *fd, void *send_fd, int *nfds );
  CTCPSocket* GetSocket() { return m_Socket; }
};
