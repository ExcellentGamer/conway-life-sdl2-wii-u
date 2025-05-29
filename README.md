# Super Mario Bros. Wii U Clone
A recreation of Super Mario Bros. (NES) on the Wii U, intended to support widescreen and other QOL Features

## Controls
- `HOME`:       Quit the Game
- `+`:          Pause/Play
- `L Stick`:    Move Mario
- `A/B`:        Jump
- `X/Y`:        Run

## Building
### Dependencies
- [wut](https://github.com/devkitPro/wut)
- SDL2 Wii U
- SDL2 Image Wii U

Install Devkitpro following [the official guide for your OS](https://devkitpro.org/wiki/Getting_Started)

Install wut:
```
(sudo) (dkp-)pacman -Syu --needed wiiu-dev
```

Install SDL2 Wii U:
```
(sudo) (dkp-)pacman -S wiiu-sdl2 wiiu-sdl2_image
```

### Compiling
Simply run `make` at the root of the repo
```
make
```

# Credits
- [BenchatonDev](https://github.com/BenchatonDev) Made the original repository I built this project on.
- [Ashquarky](https://github.com/ashquarky) For porting SDL2 to Wii U
- [Dimok](https://github.com/dimok789) For making VPADInput.h and WPADInput.h
- [GaryOderNichts](https://github.com/GaryOderNichts) For making Wii U ident which I used as 'base'
