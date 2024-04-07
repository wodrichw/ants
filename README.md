

# Build and Install
## Dependencies
Make sure you have the followng dependencies installed on your machine before installing the game
- Cmake `sudo apt install cmake -y`
- A c++ compiler `sudo apt install build-essential`
- SDL
```
sudo apt install libsdl2-dev libsdl2-2.0-0 -y
sudo apt install libjpeg-dev libwebp-dev libtiff5-dev libsdl2-image-dev libsdl2-image-2.0-0 -y
sudo apt install libmikmod-dev libfishsound1-dev libsmpeg-dev liboggz2-dev libflac-dev libfluidsynth-dev libsdl2-mixer-dev libsdl2-mixer-2.0-0 -y
sudo apt install libfreetype6-dev libsdl2-ttf-dev libsdl2-ttf-2.0-0 -y
```
- X11 `sudo apt install libx11-dev libxft-dev libxext-dev -y`
- Wayland `sudo apt install libwayland-dev libxkbcommon-dev libegl1-mesa-dev -y`
- iBus `sudo apt install libibus-1.0-dev -y`


## Install instructions
1. Get vcpkg up and running by doing `git submodule init; git submodule update; ./vcpkg/bootstrap-vcpkg.sh ; ./vcpkg/vcpkg install `
2. Get cmake to generate make file from CMakeLists.txt `[ -d "./build" ] || mkdir build; cmake -DOPT=1 ..`
3. Build ants executable `make` -- this currently builds ants with debug symbols as specified in CMakeLists.txt file

## Easy install
- run `./install_dependencies.sh`
- after this, you can build ants with `make`

