-- Now this is a real script. First, a callback is defined ...
function init()
  Controller:Register("GamePlayerChat", register_forfeit)
  forfeit_register = {}
end

-- And when a player sends a message, this function is called
function register_forfeit(event)
  -- We do nothing when the message isn't "!ff"
  if event:GetMessage() ~= "!ff" then return end
  
  -- We fetch the game, its ID, the player and his or her team
  game = event:GetGame()
  game_id = game:GetHostCounter()
  player = event:GetPlayer()
  team = game:GetTeamOfPlayer(event:GetPlayer())
  
  -- When there is no data stored for this game, initialize it
  if forfeit_register[game_id] == nil then forfeit_register[game_id] = {} end
  
  -- When there is no data stored for this team, initialize it
  if forfeit_register[game_id][team] == nil then forfeit_register[game_id][team] = {} end
  
  -- Easy access
  team_register = forfeit_register[game_id][team]
  
  -- Search through the players who have forfeited to prevent double forfeits
  found = false
  for i=0,10 do
    if team_register[i] == player:GetName() then found = true end
  end
  
  -- If the player didn't yet forfeit...
  if not found then
    -- Add him to the forfeited list
    table.insert(team_register, player:GetName())
    
    -- Update the count
    new_count = #team_register
    
    -- And fetch how many players are in the player's team
    total_count = game:GetNumPlayersInTeam(team)
    
    -- Finally, send a message to inform the players of the forfeit
    game:SendAllChat(player:GetName() .. " just forfeited. " .. tostring(new_count) .. "/" .. tostring(total_count) .. " players have forfeited so far.")
  else
    -- If the player has already forfeited, tell him that.
    game:SendAllChat("You have already forfeited, " .. player:GetName())
  end
end