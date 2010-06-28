function init()
  Controller:Register("RCCommandReceived", process_command)
  Controller:Register("GHostInitalized", save_ghost)
end

function save_ghost(event)
  ghost = event:GetGHost()
  Controller:Log("Saved GHost!")
end

function process_command(event)
  Controller:Log("Lua received " .. event:GetCommand() .. " with " .. event:GetArgCount() .. " args, the first one being " .. event:GetArg(0))
  local gamename = event:GetArg(0)
  local mapcfg = event:GetArg(1)
  local owner = event:GetArg(2)
  local public = event:GetArg(3) == "true"
  
  Controller:Log("Creating game [" .. gamename .. "]");
  Controller:Log("The config file is [" .. mapcfg .. "]");
  Controller:Log("The owner is [" .. owner .. "]");
  Controller:Log("The game is public? " .. tostring(public));
  
  if public then
    local gamestate = 16
  else
    local gamestate = 17
  end
  
  map = ghost:LoadMap(mapcfg)
  
  ghost:CreateGame(map, gamestate, false, gamename, owner, "", "", false)
end
