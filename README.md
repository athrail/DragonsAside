# Smok w bok

## Description

Simple 2D game I play with my son from time to time.
There's a board 6x8 where you have to build a road connection from one point to the other.
There's many different tiles that could be either road or dragon.

Each player on their turn draw one tile from the pile and see what it is.
If it's road then they can connect it to one of the other ends of the road.
If it's a dragon they throw two dices to check where it lands.

There's equipment on the board which can help in defending the road against the dragons.
But you need to get it first.

Good luck!

## Building

This project uses CMake for building engine as a static lib and then game that links it.
For LSP `compile_commands.json` will be generated during build.

To build on Linux use following commands:
```
cmake -B build .
cd build
cmake --build .
```

or single line to build (from root of repo):
`cmake -B build . && cmake --build build -j$(nproc)`
