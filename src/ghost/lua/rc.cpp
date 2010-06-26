#include "includes.h"
#include "socket.h"
#include "util.h"
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
#include "game_base.h"
#include "lua/rc.h"

bool CLuaRCHandler :: Update( void *fd, void *send_fd ) {
  //std::cout << "[RC] Getting new data" << std::endl;
	if( !m_Socket )
		return false;
	int before_length = (int)((*(m_Socket->GetBytes())).length());
  m_Socket->DoRecv( (fd_set *)fd );
  int after_length = (int)((*(m_Socket->GetBytes())).length());
  if(after_length != before_length)
    std::cout << "[RC] Data acquired" << std::endl;
	// extract as many packets as possible from the socket's receive buffer and put them in the m_Packets queue

	string *RecvBuffer = m_Socket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while( Bytes.size( ) >= 4 )
	{
		if( Bytes[0] == RC_HEADER_CONSTANT)
		{
		  // byte 0 is the header constant
		  // byte 1 contains the command identifier
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2);

			if( Length >= 4 )
			{
				if( Bytes.size( ) >= Length ) // the packet has been transmitted completely
				{
					m_Requests.push( new CLuaRCRequest(Bytes[1], BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length ) ) );
					std::cout << "[RC] Received valid request with command id " << Bytes[1] << std::endl;
					
					// pop package from the buffers
					*RecvBuffer = RecvBuffer->substr( Length );
					Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
				}
				else {
					return true; // packet is probably incomplete
				}
			}
			else
			{
			  // Request has invalid length
				return false;
			}
		}
		else
		{
		  // Request has no header constant
			return false;
		}
	}
	return true;
}

unsigned int CLuaRC :: SetFD( void *fd, void *send_fd, int *nfds ) {
	unsigned int NumFDs = 0;
  //std::cout << "[RC] Setting fd" << std::endl;
	if( m_Socket )
	{
		m_Socket->SetFD( (fd_set *)fd, (fd_set *)send_fd, nfds );
		NumFDs++;
	}
	
	for( vector<CLuaRCHandler *> :: iterator i = m_Handlers.begin( ); i != m_Handlers.end( ); i++ ) {
	  if((*i)->PrintInfo(fd, send_fd, nfds))
	    NumFDs++;
	  /*if((*i)->GetSocket() && !(*i)->GetSocket()->HasError()) {
  		(*i)->GetSocket( )->SetFD( (fd_set *)fd, (fd_set *)send_fd, nfds );
  			NumFDs++;
		}*/
	}
	/*
	for( vector<CLuaRCHandler *> :: iterator i = m_Handlers.begin( ); i != m_Handlers.end( ); i++ ) {
	  if(*i) {
	    CLuaRCHandler* hand = (*i);
	    CTCPSocket* csock = hand->GetSocket( );
  		if( csock )
  		{
  		  if(csock->HasError()) {
  		    std::cout << "Socket Error: " << csock->GetErrorString() << std::endl;
  		    continue;
  	    }
  		  //std::cout << "[RC] Setting for client socket..." << std::endl;
  		  
  			csock->SetFD( (fd_set *)fd, (fd_set *)send_fd, nfds );
  			//std::cout << "[RC] Set." << std::endl;
  			NumFDs++;
  		}
		}
	}*/
  //std::cout << "[RC] Finished setting" << std::endl;
	return NumFDs;
}

CLuaRCHandler :: CLuaRCHandler(CTCPSocket* n_Socket, CGHost* n_GHost) {
  std::cout << "[RC] Initializing handler, socket address " << n_Socket << std::endl; 
  m_Socket = n_Socket;
  m_GHost = n_GHost;
}

void CLuaRC :: Update( void *fd, void *send_fd ) {
  //std::cout << "[RC] Updating connections" << std::endl;
	for( vector<CLuaRCHandler *> :: iterator i = m_Handlers.begin( ); i != m_Handlers.end( ); i++ )
	  (*i)->Update(fd, send_fd);
	
	if( m_Socket ) {
  	CTCPSocket *NewSocket = m_Socket->Accept( (fd_set *)fd );
    
  	//std::cout << "[RC] Updated existing connections" << std::endl;
  	if( NewSocket )
  	{
  	  std::cout << "[RC] New connection, socket address " << NewSocket << std::endl;
  		// a new connection is waiting
    	CLuaRCHandler* hand = new CLuaRCHandler(NewSocket, m_GHost);
    	std::cout << "[RC] Created handler, socket address " << hand->GetSocket() << std::endl;
      m_Handlers.push_back(hand);
      std::cout << "[RC] Registered new connection" << std::endl;
  	}
  	//
	}
	//std::cout << "[RC] Finished updating" << std::endl;
}

CLuaRC :: CLuaRC(CGHost* n_GHost) : m_GHost(n_GHost) {
  m_Socket = new CTCPServer();
  m_Socket->Listen("", 1337);
  std::cout << "[RC] Listening for connections on port 1337" << std::endl;
}
