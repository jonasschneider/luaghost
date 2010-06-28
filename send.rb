require 'socket'      # Sockets are in standard library

RC_HEADER_CONSTANT = 209

RC_REQUEST_STATUS = 210
RC_REQUEST_GAMEINFO = 211
RC_REQUEST_CREATEGAME = 212
RC_REQUEST_LUACMD = 213

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
  def initialize(args = {})
    @sock = TCPSocket.open("localhost", 1337)
    @debug = !args[:debug].nil?
    @caching = !args[:caching].nil?
    @cache = {}
  end
  
  def close
    @sock.close
  end
  
  def tick!
    @cache = {}
  end
  
  protected
  
  def run_command(command, body = "", cacheable = true)
    if cacheable && @caching && @cache[command.to_s+body]
      return @cache[command.to_s+body]
    end
    send_command(command, body)
    reply = read_reply
    if cacheable && @caching
      @cache[command.to_s+body] = reply
    end
    reply
  end
  
  def send_command(command, body = "")
    packet = [RC_HEADER_CONSTANT, command, 0, 0]
    body.each_byte {|c| packet << c }
    
    length = [packet.length].pack("S")
    packet[2] = length[0]
    packet[3] = length[1]
    
    puts ">> #{packet.map{|b|b.to_s}.join(" ")}" if @debug
    begin
      @sock.write(packet.pack("C"*packet.size))
    rescue Exception => e
      puts e.inspect
    end
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
    resp = run_command(RC_REQUEST_CREATEGAME, body, false)
    resp.ok?
  end
  
  def luacmd(cmd, *args)
    body_parts = [cmd.to_s] + args.map(&:to_s)
    puts body_parts.inspect
    body = body_parts.pack("Z*"*body_parts.length)
    run_command(RC_REQUEST_LUACMD, body, false)
  end
end

c = GHostRCClient.new(:debug => true, :caching => true)
if c.in_lobby?
  puts "The bot is in the lobby of game '#{c.gameinfo.inspect}'"
else
  puts "The bot isn't in any lobby"
end
puts c.luacmd("testcmd", "DOTA APEM IMBA", "wormwar.cfg", "sokrates-", true).inspect
