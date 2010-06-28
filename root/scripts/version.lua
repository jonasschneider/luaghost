-- Here, a callback for GHostInitialized is registered; when GHost starts up, the below function is called.
function init()
  Controller:Register("GHostInitialized", show_version_info)
  Controller:Log("Hello from version.lua's init() function")
end

-- The function gets passed an event object. The Methods available differ for every event.
-- The definition of these events is in src/ghost/lua/clua_events.h
function show_version_info(event)
  Controller:Log("The running GHost version is: " .. tostring(event:GetGHost().version))
end