require 'socket'      # Sockets are in standard library

s = TCPSocket.open("localhost", 1337)

s.puts([209, 210, 4, 0].pack("CCCC"))

while line = s.gets   # Read lines from the socket
  puts line.chop      # And print with platform line terminator
end
