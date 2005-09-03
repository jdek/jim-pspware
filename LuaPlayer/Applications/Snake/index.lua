--[[
Snake, Copyright (c) 2005 Frank Buss <fb@frank-buss.de> (aka Shine)

   artworks by Gundrosen
   coding by Shine
   background music: "Stranglehold", composed by Jeroen Tel, (C) 1995 Maniacs of Noise
]]

-- start music
Music.playFile("stranglehold.xm", true)

-- load images
background = Image.load("background.png")
tiles = Image.load("tiles.png")
snake = Image.createEmpty(480, 272)

-- load sounds
explode = Sound.load("explode.wav")
clack = Sound.load("clack.wav")

-- define tile positions
tailR = { x = 0, y = 0 }
tailT = { x = 1, y = 0 }
tailL = { x = 2, y = 0 }
tailB = { x = 3, y = 0 }

headR = { x = 0, y = 1 }
headT = { x = 1, y = 1 }
headL = { x = 2, y = 1 }
headB = { x = 3, y = 1 }

bodyLT = { x = 0, y = 2 }
bodyLB = { x = 1, y = 2 }
bodyRT = { x = 2, y = 2 }
bodyRB = { x = 3, y = 2 }
bodyLR = { x = 4, y = 2 }
bodyTB = { x = 5, y = 2 }

apple = { x = 0, y = 3 }
fly = { x = 1, y = 3 }

-- define globals
tails = {}
heads = {}
bodies = {}

RIGHT = 0
TOP = 1
LEFT = 2
BOTTOM = 3

tails[RIGHT] = tailR
tails[TOP] = tailT
tails[LEFT] = tailL
tails[BOTTOM] = tailB

heads[RIGHT] = headR
heads[TOP] = headT
heads[LEFT] = headL
heads[BOTTOM] = headB

bodies[RIGHT] = bodyLR
bodies[TOP] = bodyTB
bodies[LEFT] = bodyLR
bodies[BOTTOM] = bodyTB

score = 0
high = 0

target = {}
targetImage = 0

dx = 0
dy = 0
cellHead = {}
cellTail = {}

function createRandomTarget()
	target.x = math.random(21)
	target.y = math.random(15)
	if math.random(2) == 1 then
		targetImage = apple
	else
		targetImage = fly
	end
end

function clearTileInSnakeImage(x, y)
	snake:fillRect(16*x, 16*y, 16, 16)
end

function drawTileToOffscreen(tile, x, y)
	screen:blit(16*x, 16*y, tiles, 16*tile.x, 16*tile.y, 16, 16)
end

function drawTileToSnakeImage(tile, x, y)
	snake:blit(16*x, 16*y, tiles, 16*tile.x, 16*tile.y, 16, 16)
end	

function newGame()
	snake:clear()
	dx = 0
	dy = 0
	x = 12
	y = 8
	cellTail.x = x
	cellTail.y = y
	cellTail.direction = RIGHT
	x = x+1
	cellHead.x = x
	cellHead.y = y
	cellHead.direction = RIGHT
	cellTail.next = cellHead
	createRandomTarget()
	drawTileToSnakeImage(tailR, cellTail.x, cellTail.y)
	drawTileToSnakeImage(headR, cellHead.x, cellHead.y)

	score = 0
end

function keyboardControl()
	screen.waitVblankStart()
	pad = Controls.read()
	if pad:up() then
		dx = 0
		dy = -1
	elseif pad:down() then
		dx = 0
		dy = 1
	elseif pad:left() then
		dx = -1
		dy = 0
	elseif pad:right() then
		dx = 1
		dy = 0
	elseif pad:circle() then
		screen:save("screenshot.tga")
	end
end

function move()
	-- do nothing, if no movement vector defined
	if dx == 0 and dy == 0 then
		return
	end
	
	-- save last head direction and position
	lastX = cellHead.x
	lastY = cellHead.y
	lastDirection = cellHead.direction

	-- add new head
	cellHead.next = {}
	cellHead = cellHead.next
	cellHead.x = lastX + dx
	cellHead.y = lastY + dy
	
	-- set direction for new head
	direction = 0
	if dx == 1 then
		direction = RIGHT
	elseif dy == -1 then
		direction = TOP
	elseif dx == -1 then
		direction = LEFT
	elseif dy == 1 then
		direction = BOTTOM
	end
	cellHead.direction = direction
	
	-- check which body to draw at the place of the old head
	clearTileInSnakeImage(lastX, lastY)
	if lastDirection == RIGHT and direction == TOP then
		drawTileToSnakeImage(bodyLT, lastX, lastY)
	elseif lastDirection == TOP and direction == LEFT then
		drawTileToSnakeImage(bodyLB, lastX, lastY)
	elseif lastDirection == LEFT and direction == BOTTOM then
		drawTileToSnakeImage(bodyRB, lastX, lastY)
	elseif lastDirection == BOTTOM and direction == RIGHT then
		drawTileToSnakeImage(bodyRT, lastX, lastY)
	elseif lastDirection == RIGHT and direction == BOTTOM then
		drawTileToSnakeImage(bodyLB, lastX, lastY)
	elseif lastDirection == TOP and direction == RIGHT then
		drawTileToSnakeImage(bodyRB, lastX, lastY)
	elseif lastDirection == LEFT and direction == TOP then
		drawTileToSnakeImage(bodyRT, lastX, lastY)
	elseif lastDirection == BOTTOM and direction == LEFT then
		drawTileToSnakeImage(bodyLT, lastX, lastY)
	else 
		drawTileToSnakeImage(bodies[direction], lastX, lastY)
	end
	
	-- draw new head
	drawTileToSnakeImage(heads[cellHead.direction], cellHead.x, cellHead.y)
	
	-- check for target
	if cellHead.x == target.x and cellHead.y == target.y then
		createRandomTarget()
		score = score + 1
		clack:play()
	else
		-- remove tail
		clearTileInSnakeImage(cellTail.x, cellTail.y)
		cellTail = cellTail.next

		-- draw new tail image
		if cellTail.direction ~= cellTail.next.direction then
			cellTail.direction = cellTail.next.direction
		end
		clearTileInSnakeImage(cellTail.x, cellTail.y)
		drawTileToSnakeImage(tails[cellTail.direction], cellTail.x, cellTail.y)
	end
end

function isGameOver()
	lastX = cellHead.x
	lastY = cellHead.y
	gameOver = false
	if lastX >= 22 then
		gameOver = true
	end
	if lastX < 1 then
		gameOver = true
	end
	if lastY >= 16 then
		gameOver = true
	end
	if lastY < 1 then
		gameOver = true
	end
	cell = cellTail
	while cell ~= cellHead do
		if cell.x == lastX and cell.y == lastY then
			gameOver = true
			break
		end
		cell = cell.next
	end
	return gameOver
end
	
-- init
math.randomseed(os.time())
newGame()

-- game loop
while not Controls.read():start() do
	for i=0,4 do
		keyboardControl()
	end
	move()
	screen:blit(0, 0, background, 0, 0, background:width(), background:height(), false)
	screen:blit(0, 0, snake)
	drawTileToOffscreen(targetImage, target.x, target.y)
	if score > high then
		high = score
	end
	screen:print(410, 81, score)
	screen:print(429, 128, high)
	screen.waitVblankStart()
	screen.flip()
	if isGameOver() then
		explode:play()
		screen.waitVblankStart(50)
		newGame()
	end
end
