/* REminiscence - Flashback interpreter
 * Copyright (C) 2005 Gregory Montoir
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <cmath>
#include "file.h"
#include "mixer.h"
#include "mod_player.h"

// XXX cleanup that mess...

Player::Player(Mixer *mixer, const char *dataPath)
	: _playing(false), _mix(mixer), _dataPath(dataPath) {
	for (int i = 0; i < 64; ++i) {
		_vibratoSineWaveform[i] = (uint8)(sin(i * 2 * M_PI / 64) * 255);
	}
}

void Player::startSong(uint8 songNum) {
	debug(DBG_MOD, "Player::startSong", songNum);
	assert(!_playing);
	if (songNum < 20) {
		if (_moduleFiles[songNum]) {
			loadModule(_moduleFiles[songNum], _dataPath);
		}
	}
}

void Player::stopSong() {
	if (_playing) {
		_playing = false;
		_mix->setPremixHook(0, 0);
		stop();
	}
}

uint16 Player::findPeriod(uint16 period, uint8 fineTune) const {
	for (int p = 0; p < 36; ++p) {
		if (_periodTable[p] == period) {
			return fineTune * 36 + p;
		}
	}
	error("Invalid period = %d", period);
	return 0;
}

void Player::loadModule(const char *filename, const char *directory) {
	File modFile;
	if (!modFile.open(filename, directory, "rb")) {
		warning("Can't open '%s'", filename);
	} else {
		modFile.read(_modInfo.songName, 20);
		_modInfo.songName[20] = 0;
		debug(DBG_MOD, "songName = '%s'", _modInfo.songName);
		
		for (int s = 0; s < NUM_SAMPLES; ++s) {
			SampleInfo *si = &_modInfo.samples[s];
			modFile.read(si->name, 22);
			si->name[22] = 0;
			si->len = modFile.readUint16BE() * 2;
			si->fineTune = modFile.readByte();
			si->volume = modFile.readByte();
			si->repeatPos = modFile.readUint16BE() * 2;
			si->repeatLen = modFile.readUint16BE() * 2;
			si->data = 0;
			debug(DBG_MOD, "sample = %d name = '%s' len = %d vol = %d", s, si->name, si->len, si->volume);
		}
		
		_modInfo.numPatterns = modFile.readByte();
		assert(_modInfo.numPatterns < NUM_PATTERNS);
		modFile.readByte(); // 0x7F
		modFile.read(_modInfo.patternOrderTable, NUM_PATTERNS);
		modFile.readUint32BE(); // 'M.K.', Protracker, 4 channels
		
		uint16 n = 0;
		for (int i = 0; i < NUM_PATTERNS; ++i) {
			if (_modInfo.patternOrderTable[i] != 0) {
				n = MAX(n, _modInfo.patternOrderTable[i]);
			}
		}
		debug(DBG_MOD, "numPatterns = %d",n + 1);
		n = (n + 1) * 64 * 4 * 4; // 64 lines of 4 notes per channel
		_modInfo.patternsTable = (uint8 *)malloc(n);
		assert(_modInfo.patternsTable);
		modFile.read(_modInfo.patternsTable, n);
		
		for (int s = 0; s < NUM_SAMPLES; ++s) {
			SampleInfo *si = &_modInfo.samples[s];
			if (si->len != 0) {
				si->data = (int8 *)malloc(si->len + 1);
				assert(si->data);
				modFile.read((int8 *)si->data, si->len);
				si->data[si->len] = si->data[si->len - 1];
			}
		}
		start();	
	}
}

void Player::start() {		
	_currentPatternOrder = 0;
	_currentPatternPos = 0;
	_currentTick = 0;
	_patternDelay = 0;
	_songSpeed = 6;
	_songTempo = 125;
	_playing = true;
	_patternLoopPos = 0;
	_patternLoopCount = -1;
	_samplesLeft = 0;
	memset(_tracks, 0, sizeof(_tracks));
	_mix->setPremixHook(mixCallback, this);
}

void Player::stop() {
	free(_modInfo.patternsTable);
	for (int s = 0; s < NUM_SAMPLES; ++s) {
		free(_modInfo.samples[s].data);
	}
	memset(&_modInfo, 0, sizeof(ModuleInfo));
}

void Player::handleNote(int trackNum, uint32 noteData) {
	Track *tk = &_tracks[trackNum];
	uint16 sampleNum = ((noteData >> 24) & 0x0F0) | ((noteData >> 12) & 0x00F);
	uint16 samplePeriod = ((noteData >> 16) & 0xF00) | ((noteData >> 16) & 0x0FF);
	uint16 effectData = noteData & 0xFFF;
	debug(DBG_MOD, "Player::handleNote(%d) p=%d/%d sampleNumber=0x%X samplePeriod=0x%X effectData=0x%X tk->period=%d", trackNum, _currentPatternPos, _currentPatternOrder, sampleNum, samplePeriod, effectData, tk->period);
	if (sampleNum != 0) {
		tk->sample = &_modInfo.samples[sampleNum - 1];
		tk->volume = tk->sample->volume;
		tk->pos = 0;
	}
	if (samplePeriod != 0) {
		tk->periodIndex = findPeriod(samplePeriod, tk->sample->fineTune);
		if ((effectData >> 8) != 0x3 && (effectData >> 8) != 0x5) {
			tk->period = _periodTable[tk->periodIndex];
			tk->freq = PAULA_FREQ / tk->period;
		} else {
			tk->portamento = _periodTable[tk->periodIndex];
		}
		tk->vibratoAmp = 0;
		tk->vibratoSpeed = 0;
		tk->vibratoPos = 0;
	}
	tk->effectData = effectData;
}

void Player::applyVolumeSlide(int trackNum, int amount) {
	debug(DBG_MOD, "Player::applyVolumeSlide(%d, %d)", trackNum, amount);
	Track *tk = &_tracks[trackNum];
	int vol = tk->volume + amount;
	if (vol < 0) {
		vol = 0;
	} else if (vol > 64) {
		vol = 64;
	}
	tk->volume = vol;
}

void Player::applyVibrato(int trackNum) {
	debug(DBG_MOD, "Player::applyVibrato(%d)", trackNum);
	Track *tk = &_tracks[trackNum];
	int vib = tk->vibratoAmp * _vibratoSineWaveform[tk->vibratoPos] / 128;
	if (tk->period + vib != 0) {
		tk->freq = PAULA_FREQ / (tk->period + vib);
	}
	tk->vibratoPos += tk->vibratoSpeed;
	if (tk->vibratoPos >= 64) {
		tk->vibratoPos = 0;
	}
}

void Player::applyPortamento(int trackNum) {
	debug(DBG_MOD, "Player::applyPortamento(%d)", trackNum);
	Track *tk = &_tracks[trackNum];
	if (tk->period < tk->portamento) {
		tk->period = MIN(tk->period + tk->portamentoSpeed, tk->portamento);
	} else if (tk->period > tk->portamento) {
		tk->period = MAX(tk->period - tk->portamentoSpeed, tk->portamento);
	}
	if (tk->period != 0) {	
		tk->freq = PAULA_FREQ / tk->period;
	}
}

void Player::handleEffect(int trackNum, bool tick) {
	Track *tk = &_tracks[trackNum];
	uint8 effectNum = tk->effectData >> 8;
	uint8 effectXY = tk->effectData & 0xFF;
	uint8 effectX = effectXY >> 4;
	uint8 effectY = effectXY & 0xF;
	debug(DBG_MOD, "Player::handleEffect(%d) effectNum=0x%X effectXY=0x%X", trackNum, effectNum, effectXY);
	switch (effectNum) {
	case 0x0: // arpeggio
		if (tick && effectXY != 0) {
			uint16 period = tk->period;
			switch (_currentTick & 3) {
			case 1:
				period = _periodTable[tk->periodIndex + effectX];
				break;
			case 2:
				period = _periodTable[tk->periodIndex + effectY];
				break;
			}
			tk->freq = PAULA_FREQ / period;
		}
		break;
	case 0x1: // portamento up
		if (tick) {
			tk->period -= effectXY;
			if (tk->period < 113) { // note B-3
				tk->period = 113;
			}
			tk->freq = PAULA_FREQ / tk->period;
		}
        	break;
    	case 0x2: // portamento down
		if (tick) {
			tk->period += effectXY;
			if (tk->period > 856) { // note C-1
				tk->period = 856;
			}
			tk->freq = PAULA_FREQ / tk->period;
		}
        	break;
	case 0x3: // tone portamento
		if (!tick) {
        	if (effectXY != 0) {
        		tk->portamentoSpeed = effectXY;
        	}
		} else {
			applyPortamento(trackNum);
		}
		break;
	case 0x4: // vibrato
		if (!tick) {
			if (effectX != 0) {
				tk->vibratoSpeed = effectX;
			}
			if (effectY != 0) {
				tk->vibratoAmp = effectY;
			}
		} else {
			applyVibrato(trackNum);
		}
		break;
	case 0x5: // tone portamento + volume slide
		if (tick) {
			applyPortamento(trackNum);
			applyVolumeSlide(trackNum, effectX - effectY);
		}	
		break;
	case 0x6: // vibrato + volume slide
		if (tick) {
			applyVibrato(trackNum);
        		applyVolumeSlide(trackNum, effectX - effectY);
		}	
		break;
	case 0x9: // set sample offset
		if (!tick) {
        		tk->pos = effectXY << (8 + FRAC_BITS);
        	}
		break;
	case 0xA: // volume slide
		if (tick) {
			applyVolumeSlide(trackNum, effectX - effectY);
		}
		break;
	case 0xB: // position jump
		if (!tick) {
			_currentPatternOrder = effectXY;
			_currentPatternPos = 0;
			assert(_currentPatternOrder < _modInfo.numPatterns);
		}
		break;
	case 0xC: // set volume
		if (!tick) {
			assert(effectXY <= 64);
			tk->volume = effectXY;
		}
		break;
	case 0xD: // pattern break
		if (!tick) {
	        	_currentPatternPos = effectX * 10 + effectY;
        		assert(_currentPatternPos < 64);
        		++_currentPatternOrder;
      		  	debug(DBG_MOD, "_currentPatternPos = %d _currentPatternOrder = %d", _currentPatternPos, _currentPatternOrder);
		}
		break;
	case 0xE: // extended effects
		switch (effectX) {
		case 0x0: // set filter, ignored
			break;
		case 0x1: // fineslide up
			if (!tick) {
				tk->period -= effectY;
				if (tk->period < 113) { // B-3 note
					tk->period = 113;
				}
				tk->freq = PAULA_FREQ / tk->period;
			}
			break;
		case 0x2: // fineslide down
			if (!tick) {
				tk->period += effectY;
				if (tk->period > 856) { // C-1 note
					tk->period = 856;
				}
				tk->freq = PAULA_FREQ / tk->period;
			}
			break;
		case 0x6: // loop pattern
			if (!tick) {
				if (effectY == 0) {
					_patternLoopPos = _currentPatternPos | (_currentPatternOrder << 8);
					debug(DBG_MOD, "_patternLoopPos = %d/%d", _currentPatternPos, _currentPatternOrder);
				} else {
					if (_patternLoopCount == -1) {
						_patternLoopCount = effectY;
						_currentPatternPos = _patternLoopPos & 0xFF;
						_currentPatternOrder = _patternLoopPos >> 8;
					} else {
						--_patternLoopCount;
						if (_patternLoopCount != 0) {
							_currentPatternPos = _patternLoopPos & 0xFF;
							_currentPatternOrder = _patternLoopPos >> 8;
						} else {
							_patternLoopCount = -1;
						}
					}
					debug(DBG_MOD, "_patternLoopCount = %d", _patternLoopCount);
				}
			}
			break;
		case 0x9: // retrigger sample
			if (tick) {
				tk->retriggerCounter = effectY;
			} else {
				if (tk->retriggerCounter == 0) {
					tk->pos = 0;
					tk->retriggerCounter = effectY;
					debug(DBG_MOD, "retrigger sample = %d _songSpeed = %d", effectY, _songSpeed);
				}
				--tk->retriggerCounter;
			}
			break;
		case 0xA: // fine volume slide up
			if (!tick) {
        		    	applyVolumeSlide(trackNum, effectY);
           		 }
			break;
		case 0xB: // fine volume slide down
			if (!tick) {
         	   		applyVolumeSlide(trackNum, -effectY);
            		}
			break;
		case 0xC: // cut sample
			if (!tick) {
				tk->cutCounter = effectY;
			} else {
				--tk->cutCounter;
				if (tk->cutCounter == 0) {
					tk->volume = 0;
				}
			}
		case 0xD: // delay sample
			if (!tick) {
				tk->delayCounter = effectY;
			} else {
				if (tk->delayCounter != 0) {
					--tk->delayCounter;
				}
			}
			break;
		case 0xE: // delay pattern
			if (!tick) {
				debug(DBG_MOD, "Player::handleEffect() _currentTick = %d delay pattern = %d", _currentTick, effectY);
				_patternDelay = effectY;
			}
			break;
		default:
			warning("Unhandled extended effect 0x%X params=0x%X", effectX, effectY);
			break;
		}
		break;
	case 0xF: // set speed
		if (!tick) {
			if (effectXY < 0x20) {
				_songSpeed = effectXY;
			} else {
				_songTempo = effectXY;
			}
		}
		break;		
	default:
		warning("Unhandled effect 0x%X params=0x%X", effectNum, effectXY);
		break;
	}
}

void Player::handleTick() {
//	if (_patternDelay != 0) {
//		--_patternDelay;
//} else 
	if (_currentTick == 0) {
		debug(DBG_MOD, "_currentPatternOrder=%d _currentPatternPos=%d", _currentPatternOrder, _currentPatternPos);
		uint8 currentPattern = _modInfo.patternOrderTable[_currentPatternOrder];
		const uint8 *p = _modInfo.patternsTable + (currentPattern * 64 + _currentPatternPos) * 16;
		for (int i = 0; i < NUM_TRACKS; ++i) {
			uint32 noteData = READ_BE_UINT32(p);
			handleNote(i, noteData);
			p += 4;
		}		
		++_currentPatternPos;
		if (_currentPatternPos == 64) {
			++_currentPatternOrder;
			_currentPatternPos = 0;
			debug(DBG_MOD, "Player::handleTick() _currentPatternOrder = %d/%d", _currentPatternOrder, _modInfo.numPatterns);
		}
	}		
	for (int i = 0; i < NUM_TRACKS; ++i) {
		handleEffect(i, (_currentTick != 0));
	}
	++_currentTick;
	if (_currentTick == _songSpeed) {
		_currentTick = 0;
	}	
	if (_currentPatternOrder == _modInfo.numPatterns) {
		debug(DBG_MOD, "Player::handleEffect() _currentPatternOrder == _modInfo.numPatterns");
		_playing = false;
	}	
}

void Player::mixSamples(int8 *buf, int samplesLen) {
	memset(buf, 0, samplesLen);
	for (int i = 0; i < NUM_TRACKS; ++i) {
		Track *tk = &_tracks[i];
		if (tk->sample != 0 && tk->delayCounter == 0) {
			int8 *mixbuf = buf;
			SampleInfo *si = tk->sample;			
			int len = si->len << FRAC_BITS;
			int loopLen = si->repeatLen << FRAC_BITS;
			int loopPos = si->repeatPos << FRAC_BITS;
			int deltaPos = (tk->freq << FRAC_BITS) / _mix->getSampleRate();
			int curLen = samplesLen;
			int pos = tk->pos;
			while (curLen != 0) {
				int count;
				if (loopLen > (2 << FRAC_BITS)) {
					assert(si->repeatPos + si->repeatLen <= si->len);					
					if (pos >= loopPos + loopLen) {
						pos -= loopLen;
					}
					count = MIN(curLen, (loopPos + loopLen - pos - 1) / deltaPos + 1);
					curLen -= count;
				} else {
					if (pos >= len) {
						count = 0;
					} else {
						count = MIN(curLen, (len - pos - 1) / deltaPos + 1);
					}
					curLen = 0;
				}
				while (count--) {
					int8 b0 = si->data[(pos >> FRAC_BITS)];
					int8 b1 = si->data[(pos >> FRAC_BITS) + 1];
					int a1 = pos & ((1 << FRAC_BITS) - 1);
					int a0 = (1 << FRAC_BITS) - a1;
					int b = (b0 * a0 + b1 * a1) >> FRAC_BITS;
					Mixer::addclamp(*mixbuf++, b * tk->volume / 64);
					pos += deltaPos;
				}
			}
			tk->pos = pos;
     	} 
	}
}

void Player::mix(int8 *buf, int len) {
	if (_playing) {
		int samplesPerTick = _mix->getSampleRate() / (50 * _songTempo / 125);
		while (len != 0) {
			if (_samplesLeft == 0) {
				handleTick();
				_samplesLeft = samplesPerTick;
			}
			int count = _samplesLeft;
			if (count > len) {
				count = len;
			}
			_samplesLeft -= count;
			len -= count;
			mixSamples(buf, count);
			buf += count;
		}
	}
}

void Player::mixCallback(void *param, int8 *buf, int len) {
	((Player *)param)->mix(buf, len);
}
