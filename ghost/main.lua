function test(ghost)
  Controller:Log(tostring(ghost.version))
end

function joined(game, player)
  game:SendAllChat("Welcome, " .. player:GetName() .. "!")
end

function init()
  Controller:Register("GHostInitialized", test)
  Controller:Register("PlayerJoined", joined)
  Controller:Log("Hallo from Lua")
end