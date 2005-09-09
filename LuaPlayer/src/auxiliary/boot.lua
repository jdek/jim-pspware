flist = System.listDirectory()
dofiles = {}

for idx, file in flist do
	if file.name ~= "." and file.name ~= ".." then
		if file.name == "SCRIPT.LUA" then -- luaplayer/script.lua
			dofiles[1] = "SCRIPT.LUA"
		end
		if file.directory then
			fflist = System.listDirectory(file.name)
			for fidx, ffile in fflist do
				if ffile.name == "SCRIPT.LUA" then -- app bundle
					dofiles[2] = file.name.."/"..ffile.name
					System.currentDirectory(file.name)
				end
				if ffile.name == "INDEX.LUA" then -- app bundle
					dofiles[2] = file.name.."/"..ffile.name
					System.currentDirectory(file.name)
				end
				
				if ffile.name == "SYSTEM.LUA" then -- luaplayer/System
					dofiles[3] = file.name.."/"..ffile.name
				end
			end
		end
	end
end
done = false
for idx, runfile in dofiles do
	dofile(runfile)
	done = true
	break
end
if not done then
	print("Boot error: No boot script found!")
end
