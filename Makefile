# Variables that may change user-to-user
SYSTEM_LIB_DIR = /usr/local/lib
SYSTEM_INCLUDE_DIR = /usr/local/include

# General variables
CXX = g++
CXXFLAGS = -std=c++17 -fPIC -I./include
BUILD_DIR = build
CPP_FILES = $(wildcard src/*.cpp) $(wildcard src/Koi/*.cpp) $(wildcard src/Koi/Bait/*.cpp)


# Debug variables
OBJ_DIR_DEBUG = $(BUILD_DIR)/debug/obj
SHARED_LIB_DEBUG = $(BUILD_DIR)/debug/libkoi.so
DEBUG_FLAGS = -g -O0
OBJ_DEBUG = $(CPP_FILES:src/%.cpp=$(OBJ_DIR_DEBUG)/%.o)

# Release variables
OBJ_DIR_RELEASE = $(BUILD_DIR)/release/obj
SHARED_LIB_RELEASE = $(BUILD_DIR)/release/libkoi.so
RELEASE_FLAGS = -Os
OBJ_RELEASE = $(CPP_FILES:src/%.cpp=$(OBJ_DIR_RELEASE)/%.o)


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
$(OBJ_DIR_DEBUG)/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Release object files
$(OBJ_DIR_RELEASE)/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@


# Debug Shared library
$(SHARED_LIB_DEBUG): $(OBJ_DEBUG)
	mkdir -p $(BUILD_DIR)/debug
	$(CXX) -shared -o $@ $^ -ltriton -lelf

# Release Shared library
$(SHARED_LIB_RELEASE): $(OBJ_RELEASE)
	mkdir -p $(BUILD_DIR)/release
	$(CXX) -shared -o $@ $^ -ltriton -lelf


# Install rule
install: $(SHARED_LIB_RELEASE)
	sudo cp $(SHARED_LIB_RELEASE) $(SYSTEM_LIB_DIR)

# Install development rule
dev: include/Koi
	@$(MAKE) install
	sudo cp -r include/Koi $(SYSTEM_INCLUDE_DIR)/


# Uninstall rule
uninstall:
	sudo rm -f $(SYSTEM_LIB_DIR)/libkoi.so
	sudo rm -rdf $(SYSTEM_INCLUDE_DIR)/Koi

# Clean rule
clean:
	rm -rf build

# Purge rule (clean and uninstall)
purge:
	@$(MAKE) uninstall
	@$(MAKE) clean
