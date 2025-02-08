# Variables that may change user-to-user
SYSTEM_LIB_DIR = /usr/local/lib
SYSTEM_INCLUDE_DIR = /usr/local/include

# General variables
CXX = g++
CXXFLAGS = -std=c++17 -fPIC -I./include
SRC = src/Elfivator.cpp src/Swimmer.cpp

# Debug variables
OBJ_DEBUG = build/debug/obj/Elfivator.o build/debug/obj/Swimmer.o
SHARED_LIB_DEBUG = build/debug/libkoi.so
DEBUG_FLAGS = -g -O0

# Release variables
OBJ_RELEASE = build/release/obj/Elfivator.o build/release/obj/Swimmer.o
SHARED_LIB_RELEASE = build/release/libkoi.so
RELEASE_FLAGS = -Os


# These rules don't create files with their name
.PHONY: clean dev purge install uninstall


# Default target is debug
all: debug


# Debug target (with -g -O0 flags)
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: $(SHARED_LIB_DEBUG)

# Release target (with -Os optimization)
release: CXXFLAGS += $(RELEASE_FLAGS)
release: $(SHARED_LIB_RELEASE)


# Debug object files
build/debug/obj/Elfivator.o: src/Elfivator.cpp
	mkdir -p build/debug/obj
	$(CXX) $(CXXFLAGS) -c $< -o $@
build/debug/obj/Swimmer.o: src/Swimmer.cpp
	mkdir -p build/debug/obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Release object files
build/release/obj/Elfivator.o: src/Elfivator.cpp
	mkdir -p build/release/obj
	$(CXX) $(CXXFLAGS) -c $< -o $@
build/release/obj/Swimmer.o: src/Swimmer.cpp
	mkdir -p build/release/obj
	$(CXX) $(CXXFLAGS) -c $< -o $@


# Debug Shared library
$(SHARED_LIB_DEBUG): $(OBJ_DEBUG)
	$(CXX) -shared -o $@ $^ -ltriton -lelf

# Release Shared library
$(SHARED_LIB_RELEASE): $(OBJ_RELEASE)
	$(CXX) -shared -o $@ $^ -ltriton -lelf


# Install rule
install: $(SHARED_LIB_RELEASE)
	sudo cp $(SHARED_LIB_RELEASE) $(SYSTEM_LIB_DIR)

# Install development rule
dev: include/Swimmer.h
	@$(MAKE) install
	sudo cp include/Swimmer.h $(SYSTEM_INCLUDE_DIR)/Swimmer.h


# Uninstall rule
uninstall:
	sudo rm -f $(SYSTEM_LIB_DIR)/libkoi.so
	sudo rm -f $(SYSTEM_INCLUDE_DIR)/Swimmer.h

# Clean rule
clean:
	rm -rf build

# Purge rule (clean and uninstall)
purge:
	@$(MAKE) uninstall
	@$(MAKE) clean
