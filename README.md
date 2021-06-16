# Plinky wavetable generator

This is a wavetable header generator for [Plinky](http://www.plinkysynth.com/).

It expects files from `c0.wav` to `c14.wav` in the same folder.

Wavs must be in this format:

- 1022 or 2044 samples
- 16 bits
- mono

The generator downsamples the waves for Plinky (32kHz).

## Building

```sh
clang++ -O2 wavetable.cpp -o wavetable
```

## Running

```sh
./wavetable
```

It should generate a `wavetable.h` which you can use in the main Plinky project- just copy it to `Core/Src/wavetable.h` and recompile the Plinky firmware.

## Credits

- Code: [@mmalex](https://twitter.com/mmalex)
- Core wavetable waves: [@miuott](https://twitter.com/miuott)

