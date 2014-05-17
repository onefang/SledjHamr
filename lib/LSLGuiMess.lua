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

TODO - Like the files window, just reuse a single window, hiding and showing it when needed.
       The switch button shows the buttons from the next one.
]]

  local dialogs = {count = 0, current = 0}
  local w = 80
  local h = 20
  local x = 2 + (((1 - 1) % 3) * w)
  local y = h * math.floor((1 - 1) / 3)
  local buttCount = 12 * 8
  local dialog = skang.window(4 + w * 3, h * math.ceil((--[[#buttons]] buttCount + 1) / 3), message, 'llDialogWindow')

  for i = 1, buttCount do
    x = 2 + (((i - 1) % 3) * w)
    y = h * math.floor((i - 1) / 3)
    skang.thingasm{dialog, 'button' .. i, 'Selects button ' .. i, types = 'widget', widget='"button", "' .. i .. '", ' .. x .. ', ' .. y ..  ', ' .. w .. ', ' .. h}
    skang.hide(dialog.W['button' .. i].Cwidget)
  end
  x = 2 + (((3 - 1) % 3) * w)
  y = h * math.floor((--[[#buttons]] buttCount + 1) / 3)
  skang.thingasm{dialog, 'ignore', 'Ignore this dialog',    types = 'widget', widget='"button", "ignore", ' .. x            .. ', ' .. y ..  ', ' .. w - 20 .. ', ' .. h}
  skang.thingasm{dialog, 'switch', 'Switch to next dialog', types = 'widget', widget='"button", ">", '      .. (x + w - 20) .. ', ' .. y ..  ', ' .. 20     .. ', ' .. h}

  llDialog = function (id, message, buttons, channel)
    local x = 2 + (((1 - 1) % 3) * w)
    local y = h * math.floor((1 - 1) / 3)

    dialogs.count = dialogs.count + 1
    dialogs[dialogs.count] = buttons
    -- Hide the last set of buttons
    if 0 ~= dialogs.current then
      for i, v in ipairs(dialogs[dialogs.current]) do
        skang.hide(dialog.W['button' .. i].Cwidget)
      end
    end
    dialogs.current = dialogs.count

    for i, v in ipairs(buttons) do
      x = 2 + (((i - 1) % 3) * w)
      y = h * math.floor((i - 1) / 3)
      skang.show(dialog.W['button' .. i].Cwidget)
      dialog.W['button' .. i].text = v
      dialog.W['button' .. i].action = 'purkle.say(' .. channel .. ', "onefang Rejected", "' .. id .. '", "' .. v .. '")'
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
