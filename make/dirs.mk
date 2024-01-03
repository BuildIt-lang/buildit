# Make helper to define all directory variables
# and sources
BASE_DIR=$(shell pwd)
SRC_DIR=$(BASE_DIR)/src
BUILD_DIR?=$(BASE_DIR)/build
INCLUDE_DIR=$(BASE_DIR)/include
SAMPLES_DIR=$(BASE_DIR)/samples
DEPS_DIR=$(BASE_DIR)/deps
SAMPLES_SRCS=$(wildcard $(SAMPLES_DIR)/*.cpp)
SAMPLES=$(subst $(SAMPLES_DIR),$(BUILD_DIR),$(SAMPLES_SRCS:.cpp=))
INCLUDES=$(wildcard $(INCLUDE_DIR)/*.h) $(wildcard $(INCLUDE_DIR)/*/*.h) $(BUILD_DIR)/gen_headers/gen/compiler_headers.h

BUILDER_SRC=$(wildcard $(SRC_DIR)/builder/*.cpp)
BLOCKS_SRC=$(wildcard $(SRC_DIR)/blocks/*.cpp)
UTIL_SRC=$(wildcard $(SRC_DIR)/util/*.cpp)

BUILDER_OBJS=$(subst $(SRC_DIR),$(BUILD_DIR),$(BUILDER_SRC:.cpp=.o))
BLOCKS_OBJS=$(subst $(SRC_DIR),$(BUILD_DIR),$(BLOCKS_SRC:.cpp=.o))
UTIL_OBJS=$(subst $(SRC_DIR),$(BUILD_DIR),$(UTIL_SRC:.cpp=.o))

LIBRARY_OBJS=$(BUILDER_OBJS) $(BLOCKS_OBJS) $(UTIL_OBJS) 
LIBRARY=$(BUILD_DIR)/lib$(LIBRARY_NAME).a

D2X_DIR?=$(BASE_DIR)/deps/d2x
D2X_BUILD_DIR?=$(D2X_DIR)/build
D2X_DEPS=$(wildcard $(D2X_DIR)/src/*) $(wildcard $(D2X_DIR)/include/*) $(wildcard $(D2X_DIR)/runtime/*)
