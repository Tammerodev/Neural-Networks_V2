# Neural Networks V2
This is based on an old project I got back at for some time.


## Installation
1. Install X11
    sudo dnf install \
    libX11-devel \
    libXrandr-devel \
    libXi-devel \
    libXcursor-devel \
    libXinerama-devel

2. Build SFML 3
    git clone https://github.com/SFML/SFML.git --branch master
    cd SFML && cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j
    sudo cmake --install build

## Usage
Run with CMake