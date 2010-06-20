function init()
  Controller:Register("GamePlayerChat", insult_player)
end

function insult_player(event)
  event:GetGame():SendAllChat("'" .. event:GetMessage() .. "'? Eh, no, " .. event:GetPlayer():GetName() .. "...")
end