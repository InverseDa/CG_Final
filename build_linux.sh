#!/bin/bash
echo -e "You are using \c"
# Check for Ubuntu
if [[ $(cat /etc/*-release 2>/dev/null | grep -i ubuntu) ]]; then
    echo "Ubuntu."

    echo "The installation had completed!"
# Check for Debian
elif [[ $(cat /etc/*-release 2>/dev/null | grep -i debian) ]]; then
    echo "Debian."
    
    sudo apt-get update
    sudo apt-get install cmake xorg-dev libglu1-mesa-dev
    wget https://github.com/glfw/glfw/releases/download/3.3.4/glfw-3.3.4.zip
    unzip glfw-3.3.4.zip

    cd glfw-3.3.4
    cmake -DBUILD_SHARED_LIBS=ON .
    make
    sudo make install

    sudo rm -rf glfw-3.3.4 glfw-3.3.4.zip
    echo "The installation had completed!"
# Check for Arch
elif [[ $(cat /etc/*-release 2>/dev/null | grep -i arch) ]]; then
    echo "Arch Linux."
    sudo pacman -Syy
    sudo pacman -S glfw assimp
    echo "The installation had completed!"
else
    echo "an unknown distribution."
    echo "Terminate the installation."
fi
