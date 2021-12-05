

FLAGS = -std=c++11 -stdlib=libc++ -Wall -Wextra
LIBS = -lglfw -lGLEW 
FRAMEWORKS = -framework CoreVideo -framework OpenGL -framework IOKit -framework Cocoa -framework Carbon 
CPPS = engine/renderer.cpp engine/openglwindow.cpp engine/buffer.cpp engine/main.cpp 

mac: 
	clang++ -c engine/pch.hpp $(FLAGS) 
	clang++ $(FLAGS) $(LIBS) $(FRAMEWORKS) -o ./output/openapp.exe $(CPPS)
	./output/openapp.exe

all: mac
