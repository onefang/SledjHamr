require "luaproc"

result, error_msg = luaproc.createworker()
if not result then print(error_msg) end
result, error_msg = luaproc.createworker()
if not result then print(error_msg) end
result, error_msg = luaproc.createworker()
if not result then print(error_msg) end
result, error_msg = luaproc.createworker()
if not result then print(error_msg) end

result, error_msg = luaproc.newproc( [=[

    local count = 0

-- Hmm, luajit 2 beta 9 crashes at about 8140, lua can go up to 50000 at least.
    for i = 1, 8000 do
	local channel = "channel" .. i
--	local proc = 'print(luaproc.receive("channel' .. i .. '") .. " " .. ' .. i ..')'
	local proc = 'luaproc.receive("' .. channel .. '")'

--	print(channel .. "\n" .. proc)
	result, error_msg = luaproc.newchannel(channel)
	if not result then print(error_msg) end
	result, error_msg = luaproc.newproc(proc)
	if not result then print(error_msg) else count = count + 1 end
    end

    print("Started " .. count .. " Lua threads")

    if 0 ~= count then
	for i = 1, count do
	    result, error_msg = luaproc.send("channel" .. i, 'luaproc is working fine! ' .. i)
	    if not result then print(error_msg .. " " .. i) end
	end
    end
]=] )
if not result then print(error_msg .. " for main proc") end

luaproc.exit()

