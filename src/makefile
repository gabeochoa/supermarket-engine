

FLAGS = -std=c++11 -stdlib=libc++
LIBS = -lglfw -lGLEW 
FRAMEWORKS = -framework CoreVideo -framework OpenGL -framework IOKit -framework Cocoa -framework Carbon 
CPPS = openglwindow.cpp main.cpp 

mac: 
	clang++ -c pch.hpp $(FLAGS) 
	clang++ $(FLAGS) $(LIBS) $(FRAMEWORKS) -o ../output/openapp.exe $(CPPS)
	../output/openapp.exe

all: mac
