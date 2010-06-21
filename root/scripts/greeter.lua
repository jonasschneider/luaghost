function init()
  Controller:Register("PlayerJoined", greet_joined_player)
end

-- Here the name of the player is used to form a chat message
function greet_joined_player(event)
  event:GetGame():SendAllChat("Welcome, " .. event:GetPlayer():GetName() .. "!")
end
