# Compiler flags
# NOTE: The -I flag is not needed if `make dev` is executed on the project
CXX = g++
CXXFLAGS = -std=c++17 -Os -I../../include

# Linking flags
# NOTE: The -L flag is not needed if `make dev` is executed on the project
LDFLAGS = -L../../build/release -lkoi -ltriton -lelf

# Example-specific
SRC = main.cpp
EXE = Key_Generation

all:
	$(MAKE) -C ../../ -f Makefile install
	$(MAKE) $(EXE)
	

$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(EXE) $(SRC) $(LDFLAGS)

clean:
	rm -f $(EXE)
