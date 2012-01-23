require "luaproc"

result, error_msg = luaproc.createworker()
if not result then print(error_msg) end
result, error_msg = luaproc.createworker()
if not result then print(error_msg) end
result, error_msg = luaproc.createworker()
if not result then print(error_msg) end
result, error_msg = luaproc.createworker()
if not result then print(error_msg) end

count = 0

proc = [=[
    require "string"
    local channel = "channel%d"
    local message

    result, error_msg = luaproc.newchannel(channel)
    if not result then print(error_msg) end
    repeat
	message = luaproc.receive(channel)
	local x, y = string.find(message, "@")
	if not x then
	x, y = string.find(message, "@")
	x, y = string.find(message, "@")
	x, y = string.find(message, "@")
	x, y = string.find(message, "@")
	x, y = string.find(message, "@")
	x, y = string.find(message, "@")
	x, y = string.find(message, "@")
	x, y = string.find(message, "@")
	end
    until "quit" == message
]=]

mainProc = [=[
    local count = %d
    local messages = 0
    local length = 0
    local message = "This is a test message. 56789 12"

    if 0 ~= count then
	for k = 0, 16 do
	    for j = 0, 199 do
		for i = 1, count do
		    result, error_msg = luaproc.send("channel" .. i, message)
		    if not result then print(error_msg .. " " .. i) else
			messages = messages + 1
			length = length + #message
		    end
		end
	    end
	message = message .. message
    end

    for i = 1, count do
	result, error_msg = luaproc.send("channel" .. i, "quit")
	if not result then print(error_msg .. " " .. i) end
	end
    end

    print("Sent " .. messages .. " messages totalling " .. length .. " bytes.  The largest message was " .. #message .. " bytes.")
]=]

for i = 1, 10 do
    result, error_msg = luaproc.newproc(string.format(proc, i))
    if not result then print(error_msg) else count = count + 1 end
end

print("Started " .. count .. " Lua threads.")

result, error_msg = luaproc.newproc(string.format(mainProc, count))
if not result then print(error_msg) end

luaproc.exit()

