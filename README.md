# boat


To build Mac:
- download SFML and place in a folder called SFML such that SFML/include exists
- run make
```
mv ~/Downloads/SFML-2.5.1-macOS-clang.tar.gz
untar -xvvf SFML-2.5.1-macOS-clang.tar.gz .
# make sure theres a folder SFML/include
make 
```

To build windows:
```
brew install mingw32
mkdir window_build
# download the GCC 7.3.0 MinGW (SEH) - 64-bit version on SFML website
mv ~/Downloads/SFML-2.5.1-windows-gcc-7.3.0-mingw-64-bit.zip windows_build/
cd windows_build/
unzip SFML-2.5.1-windows-gcc-7.3.0-mingw-64-bit.zip 
cd ../
make windows
# windows needs the dll's to run
cp windows_build/SFML-2.5.1/bin/* output/
```
