function init()
  Controller:Register("GamePlayerChat", insult_player)
  forfeit_register = {}
  gcount = 0
end

function insult_player(event)
  if event:GetMessage() ~= "!ff" then return end
  
  game = event:GetGame()
  game_id = game:GetHostCounter()
  player = event:GetPlayer()
  team = game:GetTeamOfPlayer(event:GetPlayer())
  
  if forfeit_register[game_id] == nil then forfeit_register[game_id] = {} end
  if forfeit_register[game_id][team] == nil then forfeit_register[game_id][team] = {} end
  team_register = forfeit_register[game_id][team]
  
  found = false
  for i=0,10 do
    if team_register[i] == player:GetName() then found = true end
  end
  if not found then
    table.insert(team_register, player:GetName())
    new_count = #team_register
    total_count = game:GetNumPlayersInTeam(team)
    
    game:SendAllChat(player:GetName() .. " just forfeited. " .. tostring(new_count) .. "/" .. tostring(total_count) .. " players have forfeited so far.")
  else
    game:SendAllChat("You have already forfeited, " .. player:GetName())
  end
end