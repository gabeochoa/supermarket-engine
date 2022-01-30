MAKEFLAGS := --jobs=16
MAKEFLAGS += --output-sync=target

# FLAGS = -std=c++2a -stdlib=libc++ -Wall -Wextra -g -fsanitize=address
# FLAGS = -std=c++2a -Wall -Wextra -g -I/usr/local/include -stdlib=libc++ -fsanitize=address -fsanitize=undefined
# consider adding -Wfloat-equal
FLAGS = -std=c++2a -Wall -Wextra -Wpedantic -Wuninitialized -Wshadow -Wmost -g -I/usr/local/include
LIBS = -lglfw -lglew 
FRAMEWORKS = -Ivendor/ -framework CoreVideo -framework OpenGL -framework IOKit -framework Cocoa -framework Carbon

LIB_SRC_DIR := ./engine
LIB_SRC_FILES := $(wildcard $(LIB_SRC_DIR)/*.cpp)
LIB_OBJ_DIR:= ./output/engine
LIB_OBJ_FILES := $(patsubst $(LIB_SRC_DIR)/%.cpp, $(LIB_OBJ_DIR)/%.o, $(LIB_SRC_FILES))
LIB_H_FILES := $(wildcard $(LIB_SRC_DIR)/*.h)
LIB_D_FILES := $(patsubst $(LIB_SRC_DIR)/%.h,$(LIB_OBJ_DIR)/%.d,$(LIB_SRC_FILES))
LIBRARY := ./output/libengine.a 

LIBGEN = ar
CCC = clang++
MFLAGS = -MMD -MP 


all: super

super: pch $(LIBRARY) $(OBJ_FILES)

pch: $(LIB_H_FILES)
	$(CCC) -c engine/pch.hpp -o ./output/pch.d $(FLAGS) 

$(LIBRARY): $(LIB_OBJ_FILES) 
	$(LIBGEN) rcs $(LIBRARY) $(LIB_OBJ_FILES) 

$(LIB_OBJ_DIR)/%.o: $(LIB_SRC_DIR)/%.cpp 
	$(CCC) $(FLAGS) $(MFLAGS) -c $< -o $@ 

$(LIB_OBJ_DIR)/%.d: $(LIB_SRC_DIR)/%.h
	$(CCC) $(FLAGS) $(MFLAGS) -c $< -o $@ 

clean:
	$(RM) $(LIB_OBJ_FILES) $(DEPENDS) ${LIBRARY}

.PHONY: all clean resources
