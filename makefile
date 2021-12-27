MAKEFLAGS := --jobs=16
MAKEFLAGS += --output-sync=target

# FLAGS = -std=c++2a -stdlib=libc++ -Wall -Wextra -g -fsanitize=address
# FLAGS = -std=c++2a -stdlib=libc++ -g
FLAGS = -std=c++2a -Wall -Wextra -g -I/usr/local/include
LIBS = -lglfw -lglew
FRAMEWORKS = -Ivendor/ -framework CoreVideo -framework OpenGL -framework IOKit -framework Cocoa -framework Carbon

LIB_SRC_DIR := ./engine
LIB_SRC_FILES := $(wildcard $(LIB_SRC_DIR)/*.cpp)
LIB_OBJ_DIR:= ./output/engine
LIB_OBJ_FILES := $(patsubst $(LIB_SRC_DIR)/%.cpp, $(LIB_OBJ_DIR)/%.o, $(LIB_SRC_FILES))
LIB_H_FILES := $(wildcard $(LIB_SRC_DIR)/*.h)
LIB_D_FILES := $(patsubst $(LIB_SRC_DIR)/%.h,$(LIB_OBJ_DIR)/%.d,$(LIB_SRC_FILES))
LIBRARY := ./output/libengine.a 

SRC_DIR := ./super
OBJ_DIR := ./output/super
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))
DEPENDS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.d,$(SRC_FILES))

EXE_DIR := $(OBJ_DIR)
EXE := $(OBJ_DIR)/super.exe

LIBGEN = ar
CCC = clang++
MFLAGS = -MMD -MP 

all: super

# change a bunch of vars to 
# support windows 
windows: CCC=x86_64-w64-mingw32-g++
windows: LIBGEN=x86_64-w64-mingw32-ar
windows: FLAGS=-std=c++2a -I/usr/local/include
windows: FRAMEWORKS=-Ivendor/
windows: EXE=$(OBJ_DIR)/super.windows.exe
windows: MFLAGS=
windows: super

# end windows

super: pch $(LIBRARY) $(OBJ_FILES)
	rm -f ./resources/keybindings.ini
	$(CCC) $(FLAGS) $(LIBS) $(FRAMEWORKS) -o $(EXE) ./super/main.cpp $(LIBRARY)
	DEBUG=123 ./$(EXE)

pch: $(LIB_H_FILES)
	$(CCC) -c engine/pch.hpp -o ./output/pch.d $(FLAGS) 

$(LIBRARY): $(LIB_OBJ_FILES) 
	$(LIBGEN) rcs $(LIBRARY) $(LIB_OBJ_FILES)

$(LIB_OBJ_DIR)/%.o: $(LIB_SRC_DIR)/%.cpp 
	$(CCC) $(FLAGS) $(MFLAGS) -c $< -o $@ 

$(LIB_OBJ_DIR)/%.d: $(LIB_SRC_DIR)/%.h
	$(CCC) $(FLAGS) $(MFLAGS) -c $< -o $@ 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp 
	$(CCC) $(FLAGS) $(MFLAGS) -c $< -o $@ 

clean:
	$(RM) $(OBJ_FILES) $(LIB_OBJ_FILES) $(DEPENDS) ${LIBRARY}

trace:
	rm -f super.exe.trace
	../apitracedyld/build/apitrace trace -o ./output/super.trace --api gl $(EXE)

view:
	../apitrace/build/qapitrace ./output/super.trace

.PHONY: all clean
