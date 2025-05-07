# What to Keep in Mind about Hardware

## Neopixels

How this protocol works (WS2812b):

- There is no start sequence. Just write bits. There is an end sequence though
- GRB expected. Write in little endian.
- There is no addressing. Each LED just reads first 3 bytes and forwards the rest of the message
- After the string of LEDs is written, data must be held low for a window of 50 microseconds
- WS2812 reads bits based on intervals. A 1 bit is when logic high happens ~2/3rds of the time. A 0 bit is when logic low happens ~2/3rd of the time. This can be hacked with SPI: 110 for 1, 100 for 0. 
  - See main.sysconfig for info about timing and such.

**Major Challenge 1** Find out how to write packets quickly to the LED array from MSP

## Playback

Will use audio chip for playback. Must convert .mp3 for .ogg or .wav (not recommended though) for audio chip.

**Major Challenge 2**: Find out how to have MSPM0 select song from audio chip, and have audio chip play the song on speaker. Audio chip should send signal to MSPM0 once this is done.