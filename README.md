# Euclidean

This is the repository for the code for The EUCLIDEAN synthesiser. The main sketch is in the Euclid folder, but to build it some small changes need to be made to the Teensyduino environment.

- In order to use the Ensemble effect, the two files effect_ensemble.c and effect_ensemble.h need to be added to the audio library, and effect_ensemble.h added to the Audio.h header.

- The ST7735_t3 library is used for the LCD. The fonts I use are in the Fonts folder, and need to be added to the library if not already present.

- The other libraries needed are:
	- IntervalTimer
	- Bounce2mcp
	- Adafruit_MCP23017
	- MIDI
	- USBHost_t36
	- SD_t3.h
	- ADC (only needed if for some reason the pots are particularly noisy/jittery)

- You cannot use the default Audio Library GUI with the code from Euclid.ino because it doesn't recognise AudioEffectEnsemble.  The easiest change is to make it AudioEffectChorus  instead (and change AudioConnection patchCord60(chorus, 1, effectmixR, 1) to AudioConnection patchCord60(chorus, 0, effectmixR, 1)), but remember to change it back if you make changes and want to update.

- Compile with USB type set to Serial + MIDI + Audio, and no harm overclocking to 816MHz.

- Prebuilt hex files are provided for both Teensy 4.0 and Teensy 4.1.


A User Guide for the synthesizer is here: http://thewessens.net/Euclid/Docs/User/euclideaninfo.html

A Build Guide for the kit is here: http://thewessens.net/Euclid/Docs/Build/euclideanbuild.html
