function init()
  Controller:Register("PlayerJoined", greet_joined_player)
end

function greet_joined_player(game, player)
  game:SendAllChat("Welcome, " .. player:GetName() .. "!")
end
