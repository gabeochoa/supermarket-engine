

mac: 
	clang++ -g -Wall -std=c++20 -c main.cpp 
	clang++ main.o -o ./output/main.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
	export DEBUG=123; ./output/main.exe 

windows:
	x86_64-w64-mingw32-g++ -std=c++20 -c main.cpp -I./windows_build/SFML-2.5.1/include -Wno-narrowing
	x86_64-w64-mingw32-g++ main.o -o ./output/main_win.exe -L./windows_build/SFML-2.5.1/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lwinpthread

all: mac
