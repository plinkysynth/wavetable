# Plinky wavetable generator

This is a wavetable generator for Plinky.

It expects files from `c0.wav` to `c14.wav` in the same folder.

- 1024 or 2048 samples
- 16 bits
- mono

## Building

```sh
clang++ -O2 wavetable.cpp -o wavetable
```

## Running

```sh
./wavetable
```

It should generate a `wavetable.h` which you can use in the main Plinky project.


