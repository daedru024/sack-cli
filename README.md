# Sack Cli
Group 5

## Linux (Debian/Ubuntu)

1. Install [Git](https://git-scm.com/downloads) and [CMake](https://cmake.org/download/)
2. Clone this repository
   ```
   git clone https://github.com/daedru024/sack-cli.git
   ```
3. Install SFML's dependencies
   ```
   sudo apt update
   sudo apt install \
       libxrandr-dev \
       libxcursor-dev \
       libudev-dev \
       libopenal-dev \
       libflac-dev \
       libvorbis-dev \
       libgl1-mesa-dev \
       libegl1-mesa-dev \
       libfreetype-dev
   ```
4. Change directory 
   ```
   cd sack-cli
   ```
5. Configure and build 
   ```
   cmake -B build
   cmake --build build
   ```
6. Run executable 
   ```
   ./main
   ```

## Mac

1. [Install Brew](https://brew.sh)
2. Install Git and CMake
   ```
   brew install git
   brew install cmake
   ```
3. Clone this repository 
   ```
   git clone https://github.com/daedru024/sack-cli.git
   ```
4. Change directory 
   ```
   cd sack-cli
   ```
5. Configure and build 
   ```
   cmake -B build
   cmake --build build
   ```
6. Run executable 
   ```
   ./main
   ```

## Visual Studio

1. Ensure you have the [required components installed](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio#installation).
2. Clone this repository 
   ```
   git clone https://github.com/daedru024/sack-cli.git
   ```

Visual Studio should automatically configure the CMake project, then you can build and run as normal through Visual Studio. 

## Windows
Use [WSL](https://learn.microsoft.com/en-us/windows/wsl/basic-commands)