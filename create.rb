require "send"
c = GHostRCClient.new(:debug => true)

puts c.luacmd("CreateGame", ARGV[0], "wormwar.cfg", "sokratesius", true).inspect