-- WLAN example: reads a test file from www.luaplayer.org

white = Color.new(255, 255, 255)
offscreen = Image.createEmpty(480, 272)
offscreen:clear(Color.new(0, 0, 0))
y = 0
x = 0

function graphicsPrint(text)
	for i = 1, string.len(text) do
		char = string.sub(text, i, i)
		if char == "\n" then
			y = y + 8
			x = 0
		elseif char ~= "\r" then
			offscreen:print(x, y, char, white)
			x = x + 8
		end
	end
	screen:blit(0, 0, offscreen)
	screen.waitVblankStart()
	screen.flip()
end

Wlan.init()
configs = Wlan.getConnectionConfigs()
graphicsPrint("available connections:\n")
for key, value in configs do
	graphicsPrint(key .. ": " .. value .. "\n")
end
graphicsPrint("using first connection..." .. "\n")
Wlan.useConnectionConfig(0)
-- graphicsPrint("your IP address is " .. Wlan.getIPAddress() .. "\n")   TODO: print garbage
graphicsPrint("loading test page...\n")
socket, error = Socket.connect("212.227.39.202", 80)  -- connect to www.luaplayer.org for testing
while not socket:isConnected() do System.sleep(100) end
bytesSent = socket:send("GET /wlan-test.txt HTTP/1.0\r\n")
bytesSent = socket:send("host: www.luaplayer.org\r\n\r\n")
while true do
	buffer = socket:recv()
	if string.len(buffer) > 0 then 
		graphicsPrint(buffer)
	end
	if Controls.read():start() then break end
	screen.waitVblankStart()
end
Wlan.term()
