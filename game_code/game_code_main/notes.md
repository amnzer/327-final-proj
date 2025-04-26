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
  Bits 30,29,28,27: Tracks note type (30: indicates spinner, 28: slider, 27: circle. possible 29: repeat slider but not implemented yet)
  Bit 26,25,24,23: Tracks note color (also not implemented yet)
  Bits 22,21,20: Unmapped
  Bits 19,...,0: Unsigned int; time in ms since start of song when note starts

## Incoming Technical Problems

- Quickly starting/stopping songs during a carousel
- Synchronizing the start of a song and the start of a beat
