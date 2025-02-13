# Compiler flags
# NOTE: The -I flag is not needed if `make dev` is executed on the project
C=gcc
CXX = g++
CXXFLAGS = -std=c++17 -Os -I../../include

# Linking flags
# NOTE: The -L flag is not needed if `make dev` is executed on the project
LDFLAGS = -L../../build/release -lkoi -ltriton -lelf

# Example-specific
SRC = main.cpp
EXE = String_Overflow

all:
	mkdir -p input_exe
	$(C) input_src/simple_heap.c -O0 -o input_exe/simple_heap
	$(C) input_src/simple_stack.c -O0 -o input_exe/simple_stack
	$(C) input_src/function_heap.c -O0 -o input_exe/function_heap
	$(C) input_src/function_stack.c -O0 -o input_exe/function_stack
	$(C) input_src/bad_guard.c -O0 -o input_exe/bad_guard
	$(MAKE) -C ../../ -f Makefile install
	$(MAKE) $(EXE)
	

$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(EXE) $(SRC) $(LDFLAGS)

clean:
	rm -f $(EXE)
	rm -rdf input_exe
