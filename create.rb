require "send"
c = GHostRCClient.new(true)
# void CreateGame( CMap *map, unsigned char gameState, bool saveGame, string gameName, string ownerName, string creatorName, string creatorServer, bool whisper );
# m_GHost->CreateGame( m_GHost->m_Map, GAME_PUBLIC, false, Payload, User, User, string( ), false );
c.create_game "Dota -apem imba!", RC_GAME_PUBLIC, "dota6.67c.cfg", "sokrates-"