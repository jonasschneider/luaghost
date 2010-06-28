require "send"
c = GHostRCClient.new(:debug => true)

if ARGV[0]
  puts c.luacmd("CreateGame", ARGV[0], "wormwar.cfg", "sokratesius", true).inspect
else
  puts c.luacmd("Unhost").inspect
end