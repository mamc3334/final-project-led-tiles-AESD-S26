# final-project-led-tiles-AESD-S26

This is a piano LED tiles game created for CU ECEN5713 S26.


## Current State of Game
The game is built on a Buildroot Image for the Raspberry Pi 4B. After boot, the init scripts install all necessary drivers for frame generation, audio output, server use, and game play. The game currently has a local server that can be used to dynamically load custom beatmaps and game configurations. However, some of the configuration handling is not complete, specifically for the frame generation. The current configurations that can be modified are the hit row, game speed, and scoring scale. As a follow-on to this project, the frame generator submodule could be updated to take the custom game configurations to change lane colors, and the number of players. The input handler and core game logic support 2 player use, however only one lane will be displayed on the led grid.

### Backup
In the event, that the init script fails, follow these steps to run the game from barebones buildroot image

Directions to setup for running program:

Add to config.txt (not displayed on Linux Image, to SD card writer) the following lines to enable audio and SPI
```
#enable SPI
dtparam=spi=on
#enable Audio
dtparam=audio=on
dtoverlay=audremap,enable_jack=on
```

After boot, to load sound driver with 
```
modprobe snd_bcm2835
```

Verify the audio devices with 
```
aplay -l (to check soundcards)
alsamixer (to check volume and modify sound levels)
```
