BASE_DIR=$(shell pwd)
SRC_DIR=$(BASE_DIR)/src
BUILD_DIR=$(BASE_DIR)/build
INCLUDE_DIR=$(BASE_DIR)/include
SAMPLES_DIR=$(BASE_DIR)/samples

INCLUDES=$(wildcard $(INCLUDE_DIR)/*.h) $(wildcard $(INCLUDE_DIR)/*/*.h)


$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(BUILD_DIR)/blocks)
$(shell mkdir -p $(BUILD_DIR)/builder)
$(shell mkdir -p $(BUILD_DIR)/util)
$(shell mkdir -p $(BUILD_DIR)/samples)

SAMPLES_SRCS=$(wildcard $(SAMPLES_DIR)/*.cpp)
SAMPLES=$(subst $(SAMPLES_DIR),$(BUILD_DIR),$(SAMPLES_SRCS:.cpp=))



DEBUG ?= 0
ifeq ($(DEBUG),1)
CFLAGS=-g -std=c++11
LINKER_FLAGS=-rdynamic -g
else
CFLAGS=-std=c++11 -O3
LINKER_FLAGS=-rdynamic
endif


BUILDER_SRC=$(wildcard $(SRC_DIR)/builder/*.cpp)
BLOCKS_SRC=$(wildcard $(SRC_DIR)/blocks/*.cpp)
UTIL_SRC=$(wildcard $(SRC_DIR)/util/*.cpp)

BUILDER_OBJS=$(subst $(SRC_DIR),$(BUILD_DIR),$(BUILDER_SRC:.cpp=.o))
BLOCKS_OBJS=$(subst $(SRC_DIR),$(BUILD_DIR),$(BLOCKS_SRC:.cpp=.o))
UTIL_OBJS=$(subst $(SRC_DIR),$(BUILD_DIR),$(UTIL_SRC:.cpp=.o))

LIBRARY_OBJS=$(BUILDER_OBJS) $(BLOCKS_OBJS) $(UTIL_OBJS)
LIBRARY=$(BUILD_DIR)/builder_library.a

all: executables

.PRECIOUS: $(BUILD_DIR)/builder/%.o 
.PRECIOUS: $(BUILD_DIR)/blocks/%.o 
.PRECIOUS: $(BUILD_DIR)/samples/%.o 
.PRECIOUS: $(BUILD_DIR)/util/%.o

$(BUILD_DIR)/builder/%.o: $(SRC_DIR)/builder/%.cpp $(INCLUDES)
	$(CXX) $(CFLAGS) $< -o $@ -I$(INCLUDE_DIR) -c
$(BUILD_DIR)/blocks/%.o: $(SRC_DIR)/blocks/%.cpp $(INCLUDES)
	$(CXX) $(CFLAGS) $< -o $@ -I$(INCLUDE_DIR) -c
$(BUILD_DIR)/util/%.o: $(SRC_DIR)/util/%.cpp $(INCLUDES)
	$(CXX) $(CFLAGS) $< -o $@ -I$(INCLUDE_DIR) -c


$(BUILD_DIR)/samples/%.o: $(SAMPLES_DIR)/%.cpp $(INCLUDES)
	$(CXX) $(CFLAGS) $< -o $@ -I$(INCLUDE_DIR) -c 


$(LIBRARY): $(LIBRARY_OBJS)
	ar rv $(LIBRARY) $(LIBRARY_OBJS)


$(BUILD_DIR)/sample%: $(BUILD_DIR)/samples/sample%.o $(LIBRARY)
	$(CXX) -o $@ $^ $(LINKER_FLAGS)


.PHONY: executables
executables: $(SAMPLES)

SORTED_SAMPLES=$(shell echo $(SAMPLES) | tr " " "\n" | sort -V | tr "\n" " ")


TEST ?=
run: SHELL:=/bin/bash
run: $(SAMPLES)
	@ if [ "$(TEST)" == "" ]; then \
		for sample in $(SORTED_SAMPLES); do \
			sample_name=$$(basename $$sample); \
			diff $(SAMPLES_DIR)/outputs/$$sample_name <($$sample) || exit 1; \
			echo $$sample_name: OK; \
		done \
	else \
		diff $(SAMPLES_DIR)/outputs/$(TEST) <($(BUILD_DIR)/$(TEST)) || exit 1;	\
		echo $(TEST): OK; \
	fi

clean:
	- rm -rf build
