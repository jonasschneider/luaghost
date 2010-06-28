function init()
  Controller:Register("RCCommandReceived", process_create_command)
  Controller:Register("RCCommandReceived", process_unhost_command)
  Controller:Register("GHostInitalized", save_ghost)
end

function save_ghost(event)
  ghost = event:GetGHost()
  Controller:Log("Saved GHost!")
end

function process_create_command(event)
  if event:GetCommand() ~= "CreateGame" then return end
  
  local gamename = event:GetArg(0)
  local mapcfg = event:GetArg(1)
  local owner = event:GetArg(2)
  local public = event:GetArg(3) == "true"
  
  if string.len(gamename) > 0 and string.len(mapcfg) > 0 and string.len(mapcfg) > 0 then
    Controller:Log("Creating game [" .. gamename .. "] with map " .. mapcfg .. ", owned by " .. owner .. " (public: " .. tostring(public) .. ")");
    map = ghost:LoadMap(mapcfg)
    if map then
      ghost:CreateGame(map, public, gamename, owner)
    else
      Controller:Log("Could not load map")
    end
  else
    Controller:Log("Invalid arguments passed")
  end
end

function process_unhost_command(event)
  if event:GetCommand() ~= "Unhost" then return end
  
  game = event:GetGHost().current_game
  if game then
    Controller:Log("can haz game ")
    game:Unhost()
  end
end