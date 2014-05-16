--[[  LSLGuiMess - replicates the horrid and barely usable LSL user interface crap.
]]

do

  local skang = require 'skang'
  -- This module has no default skin, it creates windows as needed.
  local _M = skang.moduleBegin('LSLGuiMess', nil, 'Copyright 2014 David Seikel', '0.1', '2014-05-16 11:07:00')
  -- This has to be global so that the action below works.
  purkle = require 'purkle'


--[[ TODO -

llTextBox()
  http://wiki.secondlife.com/wiki/LlTextBox

llSetText(string text, vector color, float alpha)
  http://lslwiki.net/lslwiki/wakka.php?wakka=llSetText

llSetSitText(string text)
  http://lslwiki.net/lslwiki/wakka.php?wakka=llSetSitText

llSetTouchText(string text)
  http://lslwiki.net/lslwiki/wakka.php?wakka=llSetTouchText

]]



--[[ llDialog(key id, string message, list buttons, integer chat_channel)
     http://lslwiki.net/lslwiki/wakka.php?wakka=llDialog
]]
llDialog = function (id, message, buttons, channel)
  local win = skang.window(200, 25 + 25 * #buttons, message, 'llDialogWindow')

print('llDialog(' .. id .. ', ' .. message .. ', , ' .. channel .. ')')
skang.printTableStart(buttons, '', 'buttons')

  for i, v in ipairs(buttons) do
    skang.thingasm{win, 'button' .. i, 'Selects button ' .. i, types = 'widget', widget='"button", "' .. v .. '", 10, ' .. (25 * i) ..  ', 60, 25'}
    win.W['button' .. i].action = 'purkle.say(' .. channel .. ', "onefang Rejected", "' .. id .. '", "' .. v .. '")'
  end

end


local doLua = function (command)
  -- Yes I know, it hurt my brain just writing this.  lol
  -- It just swaps square brackets for curly ones, coz LSL uses [] to surround lists, and Lua uses {} to surround tables.
  local c, err = loadstring(string.gsub(command, '[%[%]]', {['['] = '{', [']'] = '}'}))
  if c then
    setfenv(c, _M)
    c()
  else
    print("ERROR - " .. err)
  end

end
skang.thingasm('doLua', 'Run a Lua command.', doLua, 'string')

  skang.moduleEnd(_M)
end
