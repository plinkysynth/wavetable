# Plinky wavetable generator

This is a wavetable header generator for [Plinky](http://www.plinkysynth.com/).

It expects files from `c0.wav` to `c14.wav` in the same folder.

Wavs must be in this format:

- a single cycle waveform, of at least 512 samples (this tool resamples it to 512)
- 16 bits
- mono

The generator downsamples the waves for Plinky (32kHz) and generates various octaves pre-filtered.

## Building

```sh
clang++ -O2 wavetable.cpp -o wavetable
```

## Running

```sh
./wavetable
```

It should generate a `wavetable.h` which you can use in the main Plinky project- just copy it to `Core/Src/wavetable.h` and recompile the Plinky firmware.

### wavetab.uf2

The generator will now also generate a `wavetab.uf2` which you can just toss over to plinky! yay!

### Prebuilt binaries

There are prebuilt binaries for MacOS and Windows in the repo (`PlinkyWavetable.exe` for Windows and `wavetable` for MacOS)

## Credits

- Code: [@mmalex](https://twitter.com/mmalex)
- Core wavetable waves: [@miuott](https://twitter.com/miuott)

