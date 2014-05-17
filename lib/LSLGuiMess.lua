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
    local w = 80
    local h = 20
    local x = 2 + (((1 - 1) % 3) * w)
    local y = h * math.floor((1 - 1) / 3)
    local dialog = skang.window(4 + w * 3, h * math.ceil((#buttons + 1) / 3), message, 'llDialogWindow')

    for i, v in ipairs(buttons) do
      x = 2 + (((i - 1) % 3) * w)
      y = h * math.floor((i - 1) / 3)
      skang.thingasm{dialog, 'button' .. i, 'Selects button ' .. i, types = 'widget', widget='"button", "' .. v .. '", ' .. x .. ', ' .. y ..  ', ' .. w .. ', ' .. h}
      dialog.W['button' .. i].action = 'purkle.say(' .. channel .. ', "onefang Rejected", "' .. id .. '", "' .. v .. '")'
    end
    x = 2 + (((3 - 1) % 3) * w)
    y = h * math.floor((#buttons + 1) / 3)
    skang.thingasm{dialog, 'ignore', 'Ignore this dialog',    types = 'widget', widget='"button", "ignore", ' .. x            .. ', ' .. y ..  ', ' .. w - 20 .. ', ' .. h}
    skang.thingasm{dialog, 'switch', 'Switch to next dialog', types = 'widget', widget='"button", ">", '      .. (x + w - 20) .. ', ' .. y ..  ', ' .. 20     .. ', ' .. h}
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
