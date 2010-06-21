function init()
  Controller:Register("GamePlayerChat", insult_player)
end

-- This demonstrates the ability to "reply" to a player's chat message.
function insult_player(event)
  event:GetGame():SendAllChat("'" .. event:GetMessage() .. "'? Eh, no, " .. event:GetPlayer():GetName() .. "...")
end