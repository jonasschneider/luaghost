#include "includes.h"
#include "socket.h"
#include "ghost.h"
#include "game_base.h"
#include "map.h"
#include "config.h"
#include "util.h"
#include "gameprotocol.h"
#include "lua/rc.h"

// CLuaRCReply

BYTEARRAY CLuaRCReply :: GetPacket() {
  BYTEARRAY packet;
  packet.push_back(RC_HEADER_CONSTANT);
  if(m_Error)
    packet.push_back(m_Error);
  else
    packet.push_back(RC_RESPONSE_OK);
  packet.push_back(0);
  packet.push_back(0);

  UTIL_AppendByteArray(packet, m_Body);

	BYTEARRAY LengthBytes;
	if( packet.size( ) <= 65535 )
	{
		LengthBytes = UTIL_CreateByteArray( (uint16_t)packet.size( ), false );
		packet[2] = LengthBytes[0];
		packet[3] = LengthBytes[1];
		return packet;
	}
}



// CLuaRCClientHandler

bool CLuaRCClientHandler :: Update( void *fd, void *send_fd ) {
  m_Socket->DoRecv( (fd_set *)fd );
  ExtractPackets();
  ProcessRequests();
  m_Socket->DoSend( (fd_set *)send_fd );
}


void CLuaRCClientHandler :: ExtractPackets() {
	if( !m_Socket )
		return;
	// extract as many packets as possible from the socket's receive buffer

	string *RecvBuffer = m_Socket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes
	while( Bytes.size( ) >= 4 )
	{
		if( Bytes[0] == RC_HEADER_CONSTANT )
		{
		  // byte 0 is the header constant
		  // byte 1 contains the command identifier
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2);

			if( Length >= 4 )
			{
				if( Bytes.size( ) >= Length ) // the packet has been transmitted completely
				{
					m_Requests.push( new CLuaRCRequest(Bytes[1], BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length )) );
					std::cout << "[RC] Received request with command id " << (int)Bytes[1] << std::endl;
					
					// pop package from the buffers
					*RecvBuffer = RecvBuffer->substr( Length );
					Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
				}
				else {
					return; // packet is probably incomplete
				}
			}
			else
			{
				return; // Packet has invalid length
			}
		}
		else
		{
			return; // Packet has no header constant
		}
	}
}


void CLuaRCClientHandler :: HandleLuaCmd(BYTEARRAY Body) {
	string Command = UTIL_ByteArrayToString(UTIL_ExtractCString(Body, 0));
	if(Command.length() > 0) {
    vector<string> Args;
    string NewArg;
    int offset = Command.length() + 1;
    
    while(Body.size() > offset) {
      NewArg = UTIL_ByteArrayToString(UTIL_ExtractCString(Body, offset));
      offset += NewArg.length() + 1;
      Args.push_back(NewArg);
    }
    
    m_GHost->FireScriptEvent( new CLuaRCCommandReceivedEvent(m_GHost, this, Command, Args) );
  }
}


void CLuaRCClientHandler :: ProcessRequests() {
	while( !m_Requests.empty() )
	{
		CLuaRCRequest *Request = m_Requests.front( );
		m_Requests.pop( );
		
		BYTEARRAY ResponseBody = BYTEARRAY();
		BYTEARRAY RequestBody = Request->GetBody();
		int Error = NULL;
		
		switch( Request->GetCommandID() )
		{
		case RC_REQUEST_STATUS:
		  int Status;
		  if(!m_GHost->m_CurrentGame)
		    Status = RC_STATUS_AVAILABLE;
		  else
		    Status = RC_STATUS_BUSY;
		  ResponseBody = UTIL_CreateByteArray(Status);
		  break;
		  
		  
    case RC_REQUEST_GAMEINFO:
      if(m_GHost->m_CurrentGame) {
        UTIL_AppendByteArray(ResponseBody, m_GHost->m_CurrentGame->GetDescription( ));
        UTIL_AppendByteArray(ResponseBody, m_GHost->m_CurrentGame->GetGameName());
        UTIL_AppendByteArray(ResponseBody, m_GHost->m_CurrentGame->GetNumHumanPlayers(), false);
        UTIL_AppendByteArray(ResponseBody, m_GHost->m_CurrentGame->GetNumHumanPlayers()+m_GHost->m_CurrentGame->GetSlotsOpen(), false);

      } else
        Error = RC_RESPONSE_NOTFOUND;
    	break;
    	
    case RC_REQUEST_LUACMD:
      HandleLuaCmd(RequestBody);
      break;
      
		default:
			std::cout << "[RC] Received invalid command ID=" << Request->GetCommandID() << std::endl;
		}
		
	  SendReply(new CLuaRCReply(Request->GetCommandID(), Error, ResponseBody));
	}
}


void CLuaRCClientHandler :: SendReply(CLuaRCReply* Reply) {
  m_Socket->PutBytes(Reply->GetPacket());
}


bool CLuaRCClientHandler :: SetFD( void *fd, void *send_fd, int *nfds ) {
  if(!m_Socket) {
    std::cout << "[RC] Encountered null pointer during SetFD!" << std::endl;
    return false;
  } else {
    if(!m_Socket->GetConnected()) {
      return false;
    } else {
      if(m_Socket->HasError()) {
        std::cout << "[RC] Socket error: " << m_Socket->GetErrorString() << std::endl;
        return false;
      } else {
        m_Socket->SetFD( (fd_set *)fd, (fd_set *)send_fd, nfds );
        return true;
      }
    }
  }
}



// CLuaRC

unsigned int CLuaRC :: SetFD( void *fd, void *send_fd, int *nfds ) {
	unsigned int NumFDs = 0;
	if(m_Socket) {
		m_Socket->SetFD( (fd_set *)fd, (fd_set *)send_fd, nfds );
		NumFDs++;
	}
	
	for( vector<CLuaRCClientHandler *> :: iterator i = m_Handlers.begin( ); i != m_Handlers.end( ); ) {
	  if((*i)->SetFD(fd, send_fd, nfds)) { // all went fine
	    NumFDs++;
	    i++;
	  } else {
      std::cout << "[RC] " << (*i)->GetSocket()->GetIPString() << " disconnected" << std::endl;
	    delete *i;
	    i = m_Handlers.erase(i);
    }
  }
	return NumFDs;
}


void CLuaRC :: Update( void *fd, void *send_fd ) {
  // Update client connections
	for( vector<CLuaRCClientHandler *> :: iterator i = m_Handlers.begin( ); i != m_Handlers.end( ); i++ )
	  (*i)->Update(fd, send_fd);
	
	// Accept new connections
	if( m_Socket ) {
  	CTCPSocket *NewSocket = m_Socket->Accept( (fd_set *)fd );
    
  	if(NewSocket) {
  	  std::cout << "[RC] New connection from " << NewSocket->GetIPString() << std::endl;
      m_Handlers.push_back(new CLuaRCClientHandler(NewSocket, m_GHost));
  	}
	}
}


CLuaRC :: CLuaRC(CGHost* n_GHost) : m_GHost(n_GHost) {
  m_Socket = new CTCPServer();
  m_Socket->Listen("", 1337);
  std::cout << "[RC] Listening for connections on port 1337" << std::endl;
}
