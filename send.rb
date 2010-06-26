require 'socket'      # Sockets are in standard library

RC_HEADER_CONSTANT = 209

RC_REQUEST_STATUS = 210

RC_RESPONSE_STATUS_AVAILABLE = 230
RC_RESPONSE_STATUS_BUSY = 231

class RCReply < Struct.new(:command, :body)
  
end

class RCClient
  def initialize
    @sock = TCPSocket.open("localhost", 1337)
  end
  
  def close
    @sock.close
  end
  
  protected
    def send_command(command, body = "")
    write_command(command, body)
    read_reply
  end
  
  def write_command(command, body = "")
    packet = [RC_HEADER_CONSTANT, command, 0, 0]
    body.each_byte {|c| packet << c }
    
    length = [packet.length].pack("S")
    packet[2] = length[0]
    packet[3] = length[1]
    
    puts ">> #{packet.map{|b|b.to_s}.join(" ")}"
    @sock.write(packet.pack("C"*packet.size))
  end

  def read_reply
    throw "Invalid header constant" unless read_one  == RC_HEADER_CONSTANT
    command = read_one
    length = @sock.read(2).unpack("S")[0]
    body_length = length - 4
    body = read(body_length)
    
    reply = RCReply.new(command, body)
    puts "<< #{reply.inspect}"
    reply
  end
  
  def read_one
    read(1)[0]
  end
  
  def read(length)
    reply = @sock.read(length)
    reply.unpack("C"*reply.length).map(&:to_i)
  end
end

class GHostRCClient < RCClient
  def status
    send_command(RC_REQUEST_STATUS).body[0]
  end
  
  def available?
    status == RC_RESPONSE_STATUS_AVAILABLE
  end
end

c = GHostRCClient.new()

puts c.status.inspect
puts c.available?.inspect