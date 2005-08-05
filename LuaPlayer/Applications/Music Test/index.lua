Music.volume(96) 
SoundSystem.SFXVolume(128) -- max 
SoundSystem.reverb(4) -- out of 15 
SoundSystem.panoramicSeparation(128) -- 0 is mono

status1 = "none"
status2 = "none"

tcol = getColorNumber(0,0,64)
function render()
		clear(getColorNumber(255,255,255))
		printText(4,4,status1, tcol)
		printText(4,14,status2, tcol)
		printText(4,24,"Pan: "..pan, tcol)
		printText(4,34,"Vol: "..vol, tcol)
		printText(4,44,"Freq: "..freq, tcol)
		if voice ~= nil then
			printText(4,54,"newest voice: "..tostring(voice), tcol)
		end
		flipScreen()
end


pan, vol, freq = 128, 255, 22000


status1 = "Loading file music.xm"
render()
Music.playFile("music.xm")


status2 = "Loading sound file sound.wav"
render()
local sound = Sound.load("sound.wav") -- MUST be mono!
status2 = "Sound file loaded, soundfile="..tostring(sound)
render()


local lastkeyin = ctrlRead()
while true do
	if Music.playing() then
		status1 = "Music file playing"
	else
		status1 = "Music file not playing"
	end
	keyin = ctrlRead()
	if keyin ~= lastkeyin then
	
		if isCtrlCircle(keyin) then
			Music.playFile("music.xm")
		elseif isCtrlTriangle(keyin) then
			Music.stop()
						
		elseif isCtrlCross(keyin) then
			status2 = "Playing soundfile "..tostring(sound)
			voice = sound:play()
			voice:pan(pan)
			voice:volume(vol)
			voice:frequency(freq)
		elseif isCtrlSquare(keyin) then
			voice:stop()
		end
		
		if isCtrlHold(keyin) then -- crash.
			local a = nil
			local b = "hej"..a
		end
	end
	
	if isCtrlLTrigger(keyin) then
		pan = pan - 1
		if pan < 0 then
			pan = 0
		end
		voice:pan(pan)

	elseif isCtrlRTrigger(keyin) then
		pan = pan + 1
		if pan > 255 then
			pan = 255
		end
		voice:pan(pan)
	end
	
	if isCtrlDown(keyin) then
		vol = vol - 10
		if vol < 0 then
			vol = 0
		end
		voice:volume(vol)

	elseif isCtrlUp(keyin) then
		vol = vol + 10
		if vol > 255 then
			vol = 255
		end
		voice:volume(vol)
	end
	
	if isCtrlLeft(keyin) then
		freq = freq - 500
		if freq < 500 then
			freq = 500
		end
		voice:frequency(freq)
	elseif isCtrlRight(keyin) then
		freq = freq + 500
		if freq > 44000 then
			freq = 44000
		end
		voice:frequency(freq)
	end
	
	if isCtrlStart(keyin) then
		break
	end
	
	
	
	
	lastkeyin = keyin
	render()
	waitVblankStart()
end

