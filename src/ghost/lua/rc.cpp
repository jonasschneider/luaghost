#include "includes.h"
#include "socket.h"
#include "ghost.h"
#include "util.h"
#include "lua/rc.h"

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

void CLuaRCClientHandler :: ProcessRequests() {
	while( !m_Requests.empty() )
	{
		CLuaRCRequest *Request = m_Requests.front( );
		m_Requests.pop( );
				  uint16_t Length = 5;
  		BYTEARRAY packet;
		switch( Request->GetCommandID() )
		{
		case RC_REQUEST_STATUS:
		  int Reply;
		  if(!m_GHost->m_CurrentGame)
		    Reply = RC_RESPONSE_STATUS_AVAILABLE;
		  else
		    Reply = RC_RESPONSE_STATUS_BUSY;
		  

  		
  		packet.push_back( RC_HEADER_CONSTANT );		
    	packet.push_back( Request->GetCommandID() );
    	packet.push_back( 5 );									// packet length will be assigned later
    	packet.push_back( 0 );									// packet length will be assigned later
    	packet.push_back( Reply );
    	
    	m_Socket->PutBytes(packet);
    	
    	break;
		default:
			std::cout << "[RC] Received invalid command ID=" << Request->GetCommandID() << std::endl;
		}
	}
}

/*
bool CGameProtocol :: AssignLength( BYTEARRAY &content )
{
	// insert the actual length of the content array into bytes 3 and 4 (indices 2 and 3)

	BYTEARRAY LengthBytes;

	if( content.size( ) >= 4 && content.size( ) <= 65535 )
	{
		LengthBytes = UTIL_CreateByteArray( (uint16_t)content.size( ), false );
		content[2] = LengthBytes[0];
		content[3] = LengthBytes[1];
		return true;
	}

	return false;
}
*/

bool CLuaRCClientHandler :: SetFD( void *fd, void *send_fd, int *nfds ) {
  if(!m_Socket) {
    std::cout << "[RC] Encountered null pointer during SetFD!" << std::endl;
    return false;
  } else {
    if(!m_Socket->GetConnected()) {
      std::cout << "[RC] Connection closed" << std::endl;
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
	  } else { // There was an error, drop the connection
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
