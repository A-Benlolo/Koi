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
EXE = Double_Free

all:
	mkdir -p input_exe
	$(C) input_src/double_free.c -O0 -o input_exe/double_free
	$(MAKE) -C ../../ -f Makefile install
	$(MAKE) $(EXE)
	

$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(EXE) $(SRC) $(LDFLAGS)

clean:
	rm -f $(EXE)
	rm -rdf input_exe
