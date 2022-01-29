MAKEFLAGS := --jobs=16
MAKEFLAGS += --output-sync=target

FLAGS = -std=c++2a -Wall -Wextra -Wpedantic -Wuninitialized -Wshadow -Wmost -g -I/usr/local/include
LIBS = -lglfw -lglew 
FRAMEWORKS = -Ivendor/ -framework OpenGL -framework Cocoa 

example_name=pong

SRC_DIR := .
OBJ_DIR := ../../output/$(example_name)
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))
DEPENDS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.d,$(SRC_FILES))

EXE_DIR := $(OBJ_DIR)
EXE := $(OBJ_DIR)/$(example_name).exe

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))

CCC = clang++
MFLAGS = -MMD -MP 

all: folders $(example_name)

folders:
	mkdir -p $(OBJ_DIR)

# end windows

engine: 
	$(MAKE) -C ../..

$(example_name): engine $(OBJ_FILES)
	$(CCC) $(FLAGS) $(LIBS) $(FRAMEWORKS) -o $(EXE) ./main.cpp ../../output/libengine.a
	DEBUG=123 ./$(EXE)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp 
	$(CCC) $(FLAGS) $(MFLAGS) -c $< -o $@ 

clean:
	$(RM) $(OBJ_FILES) $(DEPENDS) 

.PHONY: all clean
