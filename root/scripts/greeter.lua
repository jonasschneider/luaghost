function init()
  Controller:Register("PlayerJoined", greet_joined_player)
end

function greet_joined_player(event)
  event:GetGame():SendAllChat("Welcome, " .. event:GetPlayer():GetName() .. "!")
end
