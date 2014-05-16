--[[  LSLGuiMess - replicates the horrid and barely usable LSL user interface crap.
]]

do

  local skang = require 'skang'
  -- This module has no default skin, it creates windows as needed.
  local _M = skang.moduleBegin('LSLGuiMess', nil, 'Copyright 2014 David Seikel', '0.1', '2014-05-16 11:07:00')

--[[ TODO -

llDialog(key id, string message, list buttons, integer chat_channel)
  http://lslwiki.net/lslwiki/wakka.php?wakka=llDialog

llTextBox()
  http://wiki.secondlife.com/wiki/LlTextBox

llSetText(string text, vector color, float alpha)
  http://lslwiki.net/lslwiki/wakka.php?wakka=llSetText

llSetSitText(string text)
  http://lslwiki.net/lslwiki/wakka.php?wakka=llSetSitText

llSetTouchText(string text)
  http://lslwiki.net/lslwiki/wakka.php?wakka=llSetTouchText

]]

  skang.moduleEnd(_M)
end

