--[[  LSLGuiMess - replicates the horrid and barely usable LSL user interface crap.
]]

do

  local skang = require 'skang'
  -- This module has no default skin, it creates windows as needed.
  local _M = skang.moduleBegin('LSLGuiMess', nil, 'Copyright 2014 David Seikel', '0.1', '2014-05-16 11:07:00')
  local purkle = require 'purkle'


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

  local dialogs = {count = 0, current = 0}
  local buttCount = 12 * 8
  local w = 80
  local h = 20
  local l = 4
  local x = 2 + (((1 - 1) % 3) * w)
  local y = h * math.floor((1 - 1) / 3)
  local dialog = skang.window(4 + w * 3, (h * l) + (h * math.ceil((buttCount + 1) / 3)), message, 'llDialogWindow')

  skang.thingasm{dialog, 'message', 'Dialog message',    types = 'widget', widget='"textbox", "", ' .. 2 .. ', ' .. 2 ..  ', ' .. (w * 3) .. ', ' .. (h * l)}
  for i = 1, buttCount do
    x = 2 + (((i - 1) % 3) * w)
    y = 2 + (h * l) + (h * math.floor((i - 1) / 3))
    skang.thingasm{dialog, 'button' .. i, 'Selects button ' .. i, types = 'widget', widget='"button", "' .. i .. '", ' .. x .. ', ' .. y ..  ', ' .. w .. ', ' .. h}
    skang.hide(dialog.W['button' .. i].Cwidget)
  end
  x = 2 + (((3 - 1) % 3) * w)
  y = 2 + (h * l) + (h * math.floor((buttCount + 1) / 3))
  -- TODO - LL's V3 has a block button to.  Dunno how I'll deal with blocking stuff, think about that later.
  -- TODO - Not sure of the point of an ignore button once I have close buttons in the window titles.
  --        Though I guess if it's showing multiple dialogs, then you could ignore individual ones.
  skang.thingasm{dialog, 'ignore', 'Ignore this dialog',    types = 'widget', widget='"button", "ignore", ' .. x            .. ', ' .. y ..  ', ' .. w - 20 .. ', ' .. h}
  dialog.W['ignore'].action = 'dialogIgnore()'
  skang.thingasm{dialog, 'switch', 'Switch to next dialog', types = 'widget', widget='"button", ">", '      .. (x + w - 20) .. ', ' .. y ..  ', ' .. 20     .. ', ' .. h}
  dialog.W['switch'].action = 'dialogSwitch()'
  skang.vanish(dialog.window)

  llDialog = function (id, message, buttons, channel)
    buttons.channel = channel
    -- TODO - Should do a llKey2Name(id) here, and let the dialogChoose function catch it so we are not waiting for it.
    --        On the other hand, this should only be used by the viewer, so only one user ever, could stash that somewhere else.
    --        On the gripping hand, does llDialog() send back as the user?  I think it does, but should check.
    buttons.name = 'onefang rejected'
    buttons.id = id
    buttons.message = message
    dialogs.count = dialogs.count + 1
    dialogs[dialogs.count] = buttons
    dialogs.current = dialogs.count
    dialogUpdate()
  end

  -- Update the current dialog.
  dialogUpdate = function ()
    local last = 1

    dialog.W['message'].text = dialogs[dialogs.current].message
    if 0 ~= dialogs.current then
      for i, v in ipairs(dialogs[dialogs.current]) do
        dialog.W['button' .. i].action = 'dialogChoose(' .. i .. ')'
        dialog.W['button' .. i].text = v
        skang.show(dialog.W['button' .. i].Cwidget)
        last = i + 1
      end
    end

    -- Hide the excess buttons
    for i = last, buttCount do
      skang.hide(dialog.W['button' .. i].Cwidget)
    end
    if dialogs.count > 1 then
      skang.show(dialog.W['switch'].Cwidget)
    else
      skang.hide(dialog.W['switch'].Cwidget)
    end
    skang.appear(dialog.window)
  end

  dialogChoose = function (i)
    local c  = dialogs[dialogs.current].channel
    local n  = dialogs[dialogs.current].name
    local id = dialogs[dialogs.current].id
    local t  = dialogs[dialogs.current][i]
    dialogIgnore()
    purkle.say(c, n, id, t)
  end

  -- Ignore button.
  dialogIgnore = function ()
    table.remove(dialogs, dialogs.current)
    dialogs.count = dialogs.count - 1
    if dialogs.current > dialogs.count then dialogs.current = dialogs.count end
    if 0 == dialogs.current then
      skang.vanish(dialog.window)
    else
      dialogUpdate()
    end
  end

  -- Switch button.
  dialogSwitch = function ()
    dialogs.current = dialogs.current + 1
    if dialogs.current > dialogs.count then dialogs.current = 1 end
    dialogUpdate()
  end


  -- TODO - This should be generalised and moved elsewhere.
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
