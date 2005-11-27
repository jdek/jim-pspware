System.usbDiskModeActivate()

Wlan.init()
configs = Wlan.getConnectionConfigs()
for key, value in configs do
	print(key .. ": " .. value)
end
Wlan.useConnectionConfig(0)
socket = Wlan.connect("203.34.166.157", 80)  -- connect to ps2dev.org for testing
socket:send("GET / HTTP/1.0\r\n\r\n")
while true do
	buffer = socket:recv()
	print(buffer)
	if Controls.read():start() then break end
	screen.waitVblankStart()
end
