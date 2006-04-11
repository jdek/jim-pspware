--[[
Game Of Life, Copyright (c) 2006 Frank Buss <fb@frank-buss.de> (aka Shine)

   demonstration for loading and using a Lua Player library
]]

-- load effect library with the lifeStep function for Game Of Life
effect = loadlib( "effect", "init" )

-- init library
effect()

-- cellular automaton dimension
width = 480
height = 272

-- create images
current = Image.createEmpty(width, height)
next = Image.createEmpty(width, height)

-- init rabbits pattern
green = Color.new(0, 255, 0)
black = Color.new(0, 0, 0)
centerX = math.floor(width / 2)
centerY = math.floor(height / 2)
current:pixel(centerX - 3, centerY, green)
current:pixel(centerX + 1, centerY, green)
current:pixel(centerX + 2, centerY, green)
current:pixel(centerX + 3, centerY, green)
current:pixel(centerX - 3, centerY + 1, green)
current:pixel(centerX - 2, centerY + 1, green)
current:pixel(centerX - 1, centerY + 1, green)
current:pixel(centerX + 2, centerY + 1, green)
current:pixel(centerX - 2, centerY + 2, green)

-- main loop
while not Controls.read():start() do
	-- calculate next generation
	Effect.lifeStep(current, next)

	-- swap buffers
	tmp = next
	next = current
	current = tmp

	-- update screen
	screen:blit(0, 0, next, 0, 0, width, height, false)
	screen.waitVblankStart()
	screen.flip()
end
