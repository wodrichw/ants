#!/bin/bash

sudo apt install cmake -y
sudo apt install build-essential -y
sudo apt install libsdl2-dev libsdl2-2.0-0 -y
sudo apt install libjpeg-dev libwebp-dev libtiff5-dev libsdl2-image-dev libsdl2-image-2.0-0 -y
sudo apt install libmikmod-dev libfishsound1-dev libsmpeg-dev liboggz2-dev libflac-dev libfluidsynth-dev libsdl2-mixer-dev libsdl2-mixer-2.0-0 -y
sudo apt install libfreetype6-dev libsdl2-ttf-dev libsdl2-ttf-2.0-0 -y
sudo apt install libx11-dev libxft-dev libxext-dev -y
sudo apt install libwayland-dev libxkbcommon-dev libegl1-mesa-dev -y
sudo apt install libibus-1.0-dev -y


git submodule init
git submodule update

./shell_scripts/setup_venv.sh
source venv/bin/activate

./lib/vcpkg/bootstrap-vcpkg.sh
./lib/vcpkg/vcpkg install


[ -d build ] || mkdir build
(cd build; cmake -DOPT=1 .. && make; )

