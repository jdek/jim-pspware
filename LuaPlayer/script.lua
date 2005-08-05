clear(getColorNumber(0,0,0))
printText(4,4,"LuaPlayer started. Loading Lowser...", getColorNumber(255,255,255))

setCurrentDirectory("Applications")
appsDir = getCurrentDirectory()

setCurrentDirectory("Lowser")
dofile("index.lua")

clear(getColorNumber(0,0,0))
printText(4,4,"Exiting LuaPlayer", getColorNumber(255,255,255))
