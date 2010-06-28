require "send"
c = GHostRCClient.new(:debug => true)

puts c.luacmd("testcmd", ARGV[0], "dota6.67c.cfg", "sokratesius", true).inspect