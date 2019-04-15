# vcmp-squirrel-mmdb [![Build status](https://ci.appveyor.com/api/projects/status/v6wyx98q2q2ekh3e?svg=true)](https://ci.appveyor.com/project/ysc3839/vcmp-squirrel-mmdb)
MaxMind DB Reader (GeoIP2) for VC:MP Squirrel Plugin.

# Building
This project uses [CMake](https://cmake.org/) build system.

## Linux
You need to install `cmake`.

On Debian-like systems just run:
`sudo apt install cmake`

After installation, run:
```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

If you want to build 32 bit binary on 64 bit OS, you need to set corresponding environment variables:
```sh
export CFLAGS="-m32"
export CXXFLAGS="-m32"
export LDFLAGS="-m32"
```

## Windows
Download & install [CMake](https://cmake.org/download/).

After installation, run:
```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
Or you can open generated solution files in Visual Studio.

The default architecture for cmake is 32 bit. If you want to build a 64 bit binary, you need to specify a 64 bit generator.

Run `cmake --help` and you can see all generators. Most cases you just need to use `Visual Studio 15 2017 Win64` generator.

For example: `cmake -G "Visual Studio 15 2017 Win64" ..`
