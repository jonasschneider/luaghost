require 'socket'      # Sockets are in standard library

RC_HEADER_CONSTANT = 209

RC_REQUEST_STATUS = 210
RC_REQUEST_GAMEINFO = 211
RC_REQUEST_CREATEGAME = 212


RC_RESPONSE_IMPOSSIBLE = 227
RC_RESPONSE_NOTFOUND = 228
RC_RESPONSE_OK = 229

RC_STATUS_AVAILABLE = 230
RC_STATUS_BUSY = 231

RC_GAME_PUBLIC = 250
RC_GAME_PRIVATE = 251

class RCReply < Struct.new(:ok, :body)
  def ok?
    ok == RC_RESPONSE_OK
  end
end

class RCClient
  def initialize(debug = false)
    @sock = TCPSocket.open("localhost", 1337)
    @debug = debug
  end
  
  def close
    @sock.close
  end
  
  protected
    def run_command(command, body = "")
    send_command(command, body)
    read_reply
  end
  
  def send_command(command, body = "")
    packet = [RC_HEADER_CONSTANT, command, 0, 0]
    body.each_byte {|c| packet << c }
    
    length = [packet.length].pack("S")
    packet[2] = length[0]
    packet[3] = length[1]
    
    puts ">> #{packet.map{|b|b.to_s}.join(" ")}" if @debug
    @sock.write(packet.pack("C"*packet.size))
  end

  def read_reply
    throw "Invalid header constant" unless read_one  == RC_HEADER_CONSTANT
    ok = read_one
    length = read(2).unpack("S")[0]
    body_length = length - 4
    body = read(body_length)
    
    reply = RCReply.new(ok, body)
    puts "<< #{reply.inspect}" if @debug
    reply
  end
  
  def read_one
    read(1)[0]
  end
  
  def read(length)
    return if length == 0
    reply = @sock.read(length)
    puts "<< #{reply.unpack("C"*reply.length).map(&:to_i).join(" ")}" if @debug
    reply
  end
end

class GHostRCClient < RCClient
  def status
    run_command(RC_REQUEST_STATUS).body
  end
  
  def available?
    status == RC_STATUS_AVAILABLE
  end
  
  def in_lobby?
    !gameinfo.nil?
  end
  
  def gameinfo
    resp = run_command(RC_REQUEST_GAMEINFO)
    if resp.ok?
      arr = resp.body.unpack("Z*Z*LL")
      { :description => arr[0], :name => arr[1], :players => arr[2], :numplayers => arr[3] }
    end
  end
  
  def create_game(gamename, gametype, mapcfg, owner)
    body = [gamename, mapcfg, owner, gametype].pack("Z*Z*Z*C")
    resp = run_command(RC_REQUEST_CREATEGAME, body)
  end
end

c = GHostRCClient.new(true)
if c.in_lobby?
  puts "The bot is in the lobby of game '#{c.gameinfo.inspect}'"
else
  puts "The bot isn't in any lobby"
end