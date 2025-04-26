# Notes about the game

## Notes about Code Operation

Each song is ~1.3kB:

- Metadata: <100kB at max.
- Hit Objects: <2kB (one int per object, up to 512 objects)

  On average, songs have ~300 objects => ~1.2kB

- Total: ~1.3kB/song average, ~2.1kB/song max

## Representing notes

- In game_code/charting/read_osu.ipynb, objects will be represented as ints. This is how:

  Bit 31: Tracks if note has been hit (dissapears if so)
  Bit 30: What channel notes are in (0=left, 1=right)
  Bits 29,28,27,26: Tracks note type (29: indicates spinner, 27: slider, 26: circle. possible 28: repeat slider but not implemented yet)
  Bit 25,24,23,22: Tracks note color (also not implemented yet)
  Bits 21,20,19: Unmapped
  Bits 18,...,0: Unsigned int; time in ms since start of song when note starts

## Incoming Technical Problems

- Quickly starting/stopping songs during a carousel
- Synchronizing the start of a song and the start of a beat
- Writing to the LED array at a good frequency (>70Hz reliably)
- Calculating time from packet number

    For audio that is sampled at 44.1k, and if you calculate ms like this: (pcm_frame\*1000)/sr, you have a max of 2\*32/44.1M = 97.4 seconds of audio you can portray. In other words you must sacrifice timing resolution (which = 1/n seconds, for pcm_frames\*n), for memory efficiency.

    Actually alternatively can just use floats instead of going for ints only...
    Or just divide sr by 4 beforehand and fix your math accordingly...
    
