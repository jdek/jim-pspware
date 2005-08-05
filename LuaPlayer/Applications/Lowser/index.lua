--[[
Lowser, Copyright (c) 2005 Joachim Bengtsson <joachimb@gmail.com> (aka Nevyn)
http://ncoder.nevyn.nu/

VERSION 0003 (v0.1)
]]

LowserView = {  
	currentIndex = 1,
	drawStartIndex = 1,
	status = "Welcome to Lowser! (c) nCoder 2005, ncoder.nevyn.nu",
	res = {
		genericFolderIcon = loadImage("img/folder.png"),
		genericAppIcon = loadImage("img/app.png"),
		genericIcon = loadImage("img/plain.png"),
		pspAppIcon = genericAppIcon,
		arrowIcon = loadImage("img/arrow.png"),
		
		backgroundImage = loadImage("img/bg.png")
	},
	colors = {
		bg = getColorNumber(255,255,255),
		text = getColorNumber(0,0,64)
	},
	contents = {},
	cwd = ""
}

function LowserView:new() -- This is just a copypaste of Shines code...
   c = {} 
   setmetatable(c, self) 
   self.__index = self 
   return c
end

function LowserView:renderIconsForCurrentDirectory()
	self.contents = dir()
	self.cwd = getCurrentDirectory()
	
	table.sort(self.contents, function (a, b) return a.name < b.name end)
	
	done = false
	while not done do -- filter out unwanted files
		for idx,file in self.contents do
			if string.sub(file.name,1,1) == "." and not string.find(file.name, "icon.png") then
				table.remove(self.contents, idx)
				break
			end
			if idx == table.getn(self.contents) then
				done = true
			end
		end
	end
	
	for idx, file in self.contents do
		-- defaults
		file.icon = nil
		file.render = nil
		file.ftype = "none"
		
		-- set icon
		if file.directory then
			-- traverse and assign correct icon and type
			setCurrentDirectory(file.name)
			subdirconts = dir()
			
			for subidx, subfile in subdirconts do
				if string.lower(subfile.name) == "index.lua" then
					file.icon = self.res.genericAppIcon
					file.ftype = "appdir"
				end
			end
			for subidx, subfile in subdirconts do
				if string.lower(subfile.name) == "icon.png" then
					file.icon = loadImage(subfile.name)
				end
			end
			setCurrentDirectory(self.cwd)
			
			if file.icon == nil then
				file.icon = self.res.genericFolderIcon
			end
			if file.ftype == "none" then
				file.ftype = "dir"
			end
		else -- is a file
			-- DEFINE AND SET FILE TYPES
			if string.lower(string.sub(file.name, -4)) == ".lua" then
				file.icon = self.res.genericAppIcon
				file.ftype = "app"
			elseif string.lower(string.sub(file.name, -4)) == ".PBP" then
				file.icon = self.res.pspAppIcon
				file.ftype = "pbp"
			else
				file.icon = self.res.genericIcon
			end
		end -- setting icon
		
		-- render
		
		file.render = createImage(192, 32)
		clear(self.colors.bg, file.render)
		
		blitAlphaImage(0,0,file.icon, file.render)

		printText(32+4, 32/2-8/2, file.name, self.colors.text, file.render)
		
	end -- loop through dir()
end -- function call

function LowserView:cd(newdir)
	self.currentIndex = 1
	self.drawStartIndex = 1
	setCurrentDirectory(newdir)
	self.cwd = getCurrentDirectory()
	self:renderIconsForCurrentDirectory() -- (hopefully expected) side effect.
end
function LowserView:render()
	local x = 14
	local y = 8
	local dirName = string.gsub(string.sub(self.cwd,5), "/", " > ")
	
	--clear(self.colors.bg)
	blitImage(0,0,self.res.backgroundImage)
	
	printText(x,y, dirName, self.colors.text)
	y = y + 8 + 14

	for i=self.drawStartIndex, math.min(self.drawStartIndex + (272-12)/36 -2, table.getn(self.contents)) do
		curfile = self.contents[i]
		if i == self.currentIndex then
			blitAlphaImage(x,y+32/2-16/2,self.res.arrowIcon)
		end
		blitAlphaImage(x+16+4, y, curfile.render)
		
		y = y + 32 + 4
	end
	
	printText(10,272-12, self.status, self.colors.text)
	
	
	flipScreen()
end

function LowserView:action(entry)
	if entry.ftype == "dir" then
		self:cd(entry.name)
		self:render()
		
	elseif entry.ftype == "appdir" then
		self.status = "Running bundle index "..entry.name.."/index.lua".."..."
		setCurrentDirectory(entry.name)
		dofile("index.lua")
		waitVblankStart(10)
		setCurrentDirectory("..")
		self:render()
		
	elseif entry.ftype == "app" then
		self.status = "Running script "..entry.name.."..."
		dofile(entry.name)
		waitVblankStart(10)
		self:render()
	else
		self.status = "Can't do anything with this file, sorry."
		self:render()
	end
end
function LowserView:filemenu(entry)
	self:action(entry) -- placeholder
end

function LowserView:event(e)
	keyin = e.value
	if isCtrlUp(keyin) then
		if self.currentIndex > 1 then
			self.currentIndex = self.currentIndex - 1
			self.drawStartIndex = math.min(self.currentIndex, self.drawStartIndex)
		end
		self:render(contents)
	elseif isCtrlDown(keyin) then
		if self.currentIndex < table.getn(self.contents) then
			self.currentIndex = self.currentIndex + 1
			if (self.currentIndex - self.drawStartIndex) > ((272-12)/36 -2) then
				self.drawStartIndex = self.drawStartIndex + 1
			end
		end
		self:render(contents)
	elseif isCtrlCircle(keyin) then
		self:filemenu(lowser.contents[self.currentIndex])
	elseif isCtrlRight(keyin) then
		if self.contents[self.currentIndex].ftype == "dir" then
			self:action(self.contents[self.currentIndex])
		else
			self:filemenu(self.contents[self.currentIndex])
		end
	elseif isCtrlLeft(keyin) or isCtrlCross(keyin) then
		self:cd("..")
		self:render()
		
	end

end


Lontroller = { 
	responders = {}
}

function Lontroller:new() -- This is just a copypaste of Shines code...
   c = {} 
   setmetatable(c, self) 
   self.__index = self 
   return c
end

function Lontroller:addResponder(responder)
	table.insert(self.responders, responder)
end

function Lontroller:dispatchEvent(e)
	self.responders[table.getn(self.responders)](e)
end






-- ================================
-- 			   Main app
-- ================================

lowser = LowserView:new() 
lowser:cd(appsDir)
lowser:render()

controller = Lontroller:new()
controller:addResponder(function (e) lowser.status = "No responder! message:"..e.value; lowser:render(); end)
controller:addResponder(function (e) lowser:event(e); end)



local lastkeyin = ctrlRead()
while true do
	keyin = ctrlRead()
	if keyin ~= lastkeyin then
		event = { type="key", value=keyin }
		controller:dispatchEvent(event)
		
		if(isCtrlStart(keyin)) then
			waitVblankStart(40)
			if isCtrlStart(ctrlRead()) then
				lowser.status = "Bye!"
				lowser:render()
				break
			end
		end
		if isCtrlTriangle(keyin) then
			screenshot(appsDir.."/../".."lowserShot.png")
		end
	end
	lastkeyin = keyin
	waitVblankStart()
end



