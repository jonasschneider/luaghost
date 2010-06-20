function show_version_info(ghost)
  Controller:Log("The running GHost version is: " .. tostring(ghost.version))
end

function init()
  Controller:Register("GHostInitialized", show_version_info)
  Controller:Log("Hello from version.lua's init() function")
end