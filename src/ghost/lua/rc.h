#include <sstream>

#define RC_HEADER_CONSTANT 209

#define RC_REQUEST_STATUS 210
#define RC_REQUEST_GAMEINFO 211
#define RC_REQUEST_LUACMD 213


#define RC_RESPONSE_IMPOSSIBLE 227
#define RC_RESPONSE_NOTFOUND 228
#define RC_RESPONSE_OK 229

#define RC_STATUS_AVAILABLE 230
#define RC_STATUS_BUSY 231

class CLuaRCReply {
  protected:
    int m_CommandID;
    BYTEARRAY m_Body;
    int m_Error;
    bool AssignLength(BYTEARRAY* packet);
  
  public:
    CLuaRCReply(int n_CommandID, int n_Error, BYTEARRAY n_Body) : m_CommandID(n_CommandID), m_Error(n_Error), m_Body(n_Body) {}
    BYTEARRAY GetPacket();
};

class CLuaRCRequest {
  protected:
    int m_CommandID;
    BYTEARRAY m_Packet;
  
  public:
    CLuaRCRequest(int n_CommandID, BYTEARRAY n_Packet) : m_CommandID(n_CommandID), m_Packet(n_Packet) {}
    int GetCommandID() { return m_CommandID; }
    BYTEARRAY GetBody() { return BYTEARRAY(m_Packet.begin() + 4, m_Packet.end()); }
};

class CLuaRCClientHandler {
  protected:
    CGHost* m_GHost;
    CTCPSocket* m_Socket;
    std::queue<CLuaRCRequest *> m_Requests;
    void ExtractPackets();
    void ProcessRequests();
    void SendReply(CLuaRCReply* Reply);
    void HandleLuaCmd(BYTEARRAY Body);
  
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
