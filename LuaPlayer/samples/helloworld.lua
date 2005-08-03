green = getColorNumber(0, 255, 0)

printText(200, 100, "Hello World!", green)

for i=0,20 do
	x0 = i/20*479
	y1 = 271-i/20*271
	drawLine(x0, 271, 479, y1, green)
end

flipScreen()

