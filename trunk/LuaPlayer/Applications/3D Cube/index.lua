System.usbDiskModeActivate()

red = Color.new(255, 0, 0)
green = Color.new(0, 255, 0)
blue = Color.new(0, 0, 255)
black = Color.new(0, 0, 0)
white = Color.new(255, 255, 255)
gray = Color.new(128, 128, 128)
cyan = Color.new(100, 255, 255)

val = 0

logo = Image.createEmpty(64, 64)
logo:clear(gray)
logo:print(20, 20, "Lua", black)
logo:print(10, 40, "Player", black)

cube = {
	{0, 0, red, -1, -1,  1},  -- 0
	{2, 0, red, -1,  1,  1},  -- 4
	{2, 2, red,  1,  1,  1},  -- 5

	{0, 0, red, -1, -1,  1},  -- 0
	{2, 2, red,  1,  1,  1},  -- 5
	{0, 2, red,  1, -1,  1},  -- 1

	{0, 0, red, -1, -1, -1},  -- 3
	{2, 0, red,  1, -1, -1},  -- 2
	{2, 2, red,  1,  1, -1},  -- 6

	{0, 0, red, -1, -1, -1},  -- 3
	{2, 2, red,  1,  1, -1},  -- 6
	{0, 2, red, -1,  1, -1},  -- 7

	{0, 0, green,  1, -1, -1},  -- 0
	{2, 0, green,  1, -1,  1},  -- 3
	{2, 2, green,  1,  1,  1},  -- 7

	{0, 0, green,  1, -1, -1},  -- 0
	{2, 2, green,  1,  1,  1},  -- 7
	{0, 2, green,  1,  1, -1},  -- 4

	{0, 0, green, -1, -1, -1},  -- 0
	{2, 0, green, -1,  1, -1},  -- 3
	{2, 2, green, -1,  1,  1},  -- 7

	{0, 0, green, -1, -1, -1},  -- 0
	{2, 2, green, -1,  1,  1},  -- 7
	{0, 2, green, -1, -1,  1},  -- 4

	{0, 0, blue, -1,  1, -1},  -- 0
	{2, 0, blue,  1,  1, -1},  -- 1
	{2, 2, blue,  1,  1,  1},  -- 2

	{0, 0, blue, -1,  1, -1},  -- 0
	{2, 2, blue,  1,  1,  1},  -- 2
	{0, 2, blue, -1,  1,  1},  -- 3

	{0, 0, blue, -1, -1, -1},  -- 4
	{2, 0, blue, -1, -1,  1},  -- 7
	{2, 2, blue,  1, -1,  1},  -- 6

	{0, 0, blue, -1, -1, -1},  -- 4
	{2, 2, blue,  1, -1,  1},  -- 6
	{0, 2, blue,  1, -1, -1},  -- 5
}

plane = {
	{blue, -8, -3,  0},
	{cyan, 8,  3,  0},
	{blue,  8,  -3,  0},
	{cyan, 8, 3,  0},
	{blue, -8,  -3,  0},
	{cyan,  -8,  3,  0},
}

while true do
	start3d()

	-- clear screen
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT)

	-- setup projection and view matrices

	sceGumMatrixMode(GU_PROJECTION)
	sceGumLoadIdentity()
	sceGumPerspective(75, 16/9, 0.5, 1000)

	sceGumMatrixMode(GU_VIEW)
	sceGumLoadIdentity()

	-- setup matrix for triangle
	sceGumMatrixMode(GU_MODEL)
	sceGumLoadIdentity()
	sceGumTranslate(0, 0, -3);

	-- draw triangle without texture

	sceGuDisable(GU_TEXTURE_2D)
	sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D, plane)

	-- setup texture

	sceGuEnable(GU_BLEND)
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0)
	sceGuEnable(GU_TEXTURE_2D);
	sceGuTexImage(logo)
	sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA)
	sceGuTexEnvColor(white)
	sceGuTexFilter(GU_LINEAR, GU_LINEAR)
	sceGuTexScale(1, 1)
	sceGuTexOffset(0, 0)
	sceGuAmbientColor(white)

	-- setup matrix for cube

	sceGumMatrixMode(GU_MODEL)
	sceGumLoadIdentity()
	sceGumTranslate(0, 0, -3.5);
	sceGumRotateXYZ(val * 0.79 * (GU_PI/180), val * 0.98 * (GU_PI/180.0), val * 1.32 * (GU_PI/180.0))

	-- draw cube

	sceGumDrawArray(GU_TRIANGLES, GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D, cube)

	end3d()

	val = val + 1

	if Controls.read():start() then
		screen:clear()
		break
	end

	screen.waitVblankStart()
	screen.flip()
end
