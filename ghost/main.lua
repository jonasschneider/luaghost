function test(ghost)
  Controller:Log(tostring(ghost.version))
end

function init()
  Controller:Register("GHostInitialized", test)
  Controller:Log("Hallo from Lua")
end