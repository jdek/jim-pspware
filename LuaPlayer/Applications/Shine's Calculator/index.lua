keys = {
	{ "C", "sin", "cos", "tan" },
	{ "1/x", "x^2", "sqr", "/" },
	{ "7", "8", "9", "*" },
	{ "4", "5", "6", "-" },
	{ "1", "2", "3", "+" },
	{ "0", "+/-", ".", "=" } }

color = getColorNumber(0, 255, 0)

function drawRect(x0, y0, w, h)
	drawLine(x0, y0, x0+w, y0, color)
	drawLine(x0, y0, x0, y0+h, color)
	drawLine(x0+w, y0, x0+w, y0+h, color)
	drawLine(x0+w, y0+h, x0, y0+h, color)
end

oldPad = 0
x = 1
y = 1
text = "0"
lastNumber = 0
deleteOnKey = true
lastOperation = ""
value = ""
number = 0

while true do
	pad = ctrlRead()
	if pad ~= oldPad then
		if isCtrlTriangle(pad) then
			screenshot("calculator.tga")
		end
		
		if isCtrlStart(pad) then
			break
		end

		if isCtrlLeft(pad) then
			x = x - 1
			if x == 0 then
				x = 4
			end
		end
		if isCtrlRight(pad) then
			x = x + 1
			if x == 5 then
				x = 1
			end
		end
		if isCtrlUp(pad) then
			y = y - 1
			if y == 0 then
				y = 6
			end
		end
		if isCtrlDown(pad) then
			y = y + 1
			if y == 7 then
				y = 1
			end
		end
		if isCtrlCross(pad) then
			value = keys[y][x]
			number = tonumber(text)
			if value == "C" then
				text = "0"
				lastNumber = 0
				lastOperation = ""
				deleteOnKey = true
			elseif value == "1/x" then
				text = tostring(1/number)
			elseif value == "sin" then
				text = tostring(math.sin(number))
			elseif value == "cos" then
				text = tostring(math.cos(number))
			elseif value == "tan" then
				text = tostring(math.tan(number))
			elseif value == "x^2" then
				text = tostring(number*number)
			elseif value == "sqr" then
				text = tostring(math.sqrt(number))
			elseif value == "/" or value == "*" or value == "-" or value == "+" or value == "=" then
				if lastOperation == "/" then
					text = tostring(lastNumber / number)
				elseif lastOperation == "*" then
					text = tostring(lastNumber * number)
				elseif lastOperation == "-" then
					text = tostring(lastNumber - number)
				elseif lastOperation == "+" then
					text = tostring(lastNumber + number)
				end
				number = tonumber(text)
				if value == "=" then
					text = tostring(number)
					lastOperation = ""
				else
					lastNumber = number
					lastOperation = value
				end
				deleteOnKey = true
			elseif value == "+/-" then
				text = tostring(-number)
			else
				if deleteOnKey then
					text = value
					deleteOnKey = false
				else
					text = text .. value
				end
			end
		end
		oldPad = pad
	end
	
	clear(0)
	yk = 0
	w = 40
	h = 30
	drawRect(w, 0, w * 4, h-2)
	printText(w+4, 4, text, color)
	for yindex,line in keys do
		for xindex,key in line do
			x0 = xindex * w
			y0 = yindex * h
			drawRect(x0, y0, w, h)
			if xindex == x and yindex == y then
				fillRect(color, x0, y0, w, h)
				foreground = 0
			else
				foreground = color
			end
			printText(x0 + 5, y0 + 5, key, foreground)
		end
	end
	waitVblankStart()
	flipScreen()
end
