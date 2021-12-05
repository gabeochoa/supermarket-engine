
FLAGS = -std=c++11 -stdlib=libc++ -Wall -Wextra
LIBS = -lglfw -lGLEW 
FRAMEWORKS = -framework CoreVideo -framework OpenGL -framework IOKit -framework Cocoa -framework Carbon
SRC_DIR := ./engine
OBJ_DIR := ./output
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))
LIBRARY := ./output/libengine.a


all: pch $(LIBRARY) super

pch:
	clang++ -c engine/pch.hpp $(FLAGS) 

$(LIBRARY): $(OBJ_FILES) 
	ar rcs $(LIBRARY) $(OBJ_FILES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	clang++ $(FLAGS) -c -o $@ $<


super: $(LIBRARY)
	clang++ $(FLAGS) $(LIBS) $(FRAMEWORKS) -o ./output/super.exe ./super/main.cpp ./output/libengine.a
	./output/super.exe

mac:
	clang++ -c engine/pch.hpp $(FLAGS) 
	clang++ $(FLAGS) $(LIBS) $(FRAMEWORKS) -o ./output/openapp.exe $(CPPS)
	./output/openapp.exe


