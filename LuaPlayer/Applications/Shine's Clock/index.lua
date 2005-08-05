background = createImage(480, 272) 
clockOfs = 150 
clockWidth = 100 
clockTextPosition = 85 
clockBigMarkWidth = 7 
clockSmallMarkWidth = 3 
x0 = clockOfs 
y0 = clockOfs - clockWidth 
pi = 4*math.atan(1) 
color = getColorNumber(0, 255, 0) 
for i=0,60 do 
   x1 = math.sin(pi-i/60*2*pi) * clockWidth + clockOfs 
   y1 = math.cos(pi-i/60*2*pi) * clockWidth + clockOfs 
   drawLine(x0, y0, x1, y1, color, background) 
   xv = (x1 - clockOfs) / clockWidth 
   yv = (y1 - clockOfs) / clockWidth 
   if math.mod(i, 5) == 0 then 
      xt = xv * clockTextPosition + clockOfs 
      yt = yv * clockTextPosition + clockOfs 
      value = math.ceil(i / 5) 
      if value == 0 then 
         value = 12 
      end 
      printDecimal(xt, yt, value, color, background) 
      xv = xv * (clockWidth - clockBigMarkWidth) + clockOfs 
      yv = yv * (clockWidth - clockBigMarkWidth) + clockOfs 
      drawLine(x1, y1, xv, yv, color, background) 
   else 
      xv = xv * (clockWidth - clockSmallMarkWidth) + clockOfs 
      yv = yv * (clockWidth - clockSmallMarkWidth) + clockOfs 
      drawLine(x1, y1, xv, yv, color, background) 
   end 
   x0 = x1 
   y0 = y1 
end 
printText(4, 4, "os.date: ", color, background) 
printText(4, 14, "digital: ", color, background) 
waitVblankStart(10) 
lastinput = ctrlRead()
while lastinput == ctrlRead() do 
   blitImage(0, 0, background) 
   time = os.time() 
   dateString = os.date("%c", time) 
   printText(84, 4, dateString, color) 
   dateFields = os.date("*t", time) 
   printDecimal(84, 14, dateFields.hour, color) 
   printText(96, 13, ":", getColorNumber(0, 255, 0)) 
   printDecimal(102, 14, dateFields.min, color) 
   printText(112, 13, ":", getColorNumber(0, 255, 0)) 
   printDecimal(118, 14, dateFields.sec, color) 

   hour = dateFields.hour 
   if hour > 12 then 
      hour = hour - 12 
   end 
   hour = hour + dateFields.min / 60 + dateFields.sec / 3600 
   x = math.sin(pi-hour/12*2*pi) * clockWidth / 3 * 2 + clockOfs 
   y = math.cos(pi-hour/12*2*pi) * clockWidth / 3 * 2 + clockOfs 
   drawLine(clockOfs, clockOfs, x, y, color) 

   min = dateFields.min + dateFields.sec / 60 
   x = math.sin(pi-min/60*2*pi) * clockWidth + clockOfs 
   y = math.cos(pi-min/60*2*pi) * clockWidth + clockOfs 
   drawLine(clockOfs, clockOfs, x, y, color) 

   x = math.sin(pi-dateFields.sec/60*2*pi) * clockWidth + clockOfs 
   y = math.cos(pi-dateFields.sec/60*2*pi) * clockWidth + clockOfs 
   drawLine(clockOfs, clockOfs, x, y, color) 

   waitVblankStart() 
   flipScreen() 
end 