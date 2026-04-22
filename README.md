# final-project-led-tiles-AESD-S26

This is a piano LED tiles game created for CU ECEN5713 S26.

Further changes required to run the game from barebones buildroot image


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