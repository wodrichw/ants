#!/bin/bash

export apt_install_stuff=0
if [[ $apt_install_stuff == "1" ]] ; then
    sudo apt install cmake -y
    sudo apt install build-essential -y
    sudo apt install libsdl2-dev libsdl2-2.0-0 -y
    sudo apt install libjpeg-dev libwebp-dev libtiff5-dev libsdl2-image-dev libsdl2-image-2.0-0 -y
    sudo apt install libmikmod-dev libfishsound1-dev libsmpeg-dev liboggz2-dev libflac-dev libfluidsynth-dev libsdl2-mixer-dev libsdl2-mixer-2.0-0 -y
    sudo apt install libfreetype6-dev libsdl2-ttf-dev libsdl2-ttf-2.0-0 -y
    sudo apt install libx11-dev libxft-dev libxext-dev -y
    sudo apt install libwayland-dev libxkbcommon-dev libegl1-mesa-dev -y
    sudo apt install libibus-1.0-dev -y
    sudo apt install autoconf -y
    sudo apt install libtool -y
    sudo apt install python3.11-dev -y
fi

git submodule init
git submodule update

./lib/vcpkg/bootstrap-vcpkg.sh
./lib/vcpkg/vcpkg install

[ -d build ] || mkdir build
(cd build; rm -rf * ; cmake -DOPT=1 .. && make; )

mv .githooks/pre-commit .git/hooks/
