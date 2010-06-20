function show_version_info(event)
  Controller:Log("The running GHost version is: " .. tostring(event:GetGHost().version))
end

function init()
  Controller:Register("GHostInitialized", show_version_info)
  Controller:Log("Hello from version.lua's init() function")
end