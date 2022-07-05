LIBRARY_NAME=buildit
BASE_DIR=$(shell pwd)
SRC_DIR=$(BASE_DIR)/src
BUILD_DIR?=$(BASE_DIR)/build
INCLUDE_DIR=$(BASE_DIR)/include
SAMPLES_DIR=$(BASE_DIR)/samples
DEPS_DIR=$(BASE_DIR)/deps

INCLUDES=$(wildcard $(INCLUDE_DIR)/*.h) $(wildcard $(INCLUDE_DIR)/*/*.h) $(BUILD_DIR)/gen_headers/gen/compiler_headers.h


$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(BUILD_DIR)/blocks)
$(shell mkdir -p $(BUILD_DIR)/builder)
$(shell mkdir -p $(BUILD_DIR)/util)
$(shell mkdir -p $(BUILD_DIR)/samples)
$(shell mkdir -p $(BUILD_DIR)/gen_headers)
$(shell mkdir -p $(BUILD_DIR)/gen_headers/gen)
$(shell mkdir -p $(BASE_DIR)/scratch)


SAMPLES_SRCS=$(wildcard $(SAMPLES_DIR)/*.cpp)
SAMPLES=$(subst $(SAMPLES_DIR),$(BUILD_DIR),$(SAMPLES_SRCS:.cpp=))

RECOVER_VAR_NAMES ?= 0
ifeq ($(RECOVER_VAR_NAMES),1)
DEBUG=1
endif


DEBUG ?= 0
ifeq ($(DEBUG),1)
CFLAGS=-g -std=c++11
LINKER_FLAGS=-l$(LIBRARY_NAME) -g
else
CFLAGS=-std=c++11 -O3
LINKER_FLAGS=-l$(LIBRARY_NAME)
endif

TRACER_USE_FLIMITS ?= 0
ifeq ($(TRACER_USE_FLIMITS),1)
CFLAGS+=-DTRACER_USE_FLIMITS
else
LINKER_FLAGS+=-rdynamic
endif


CFLAGS+=-Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wmissing-declarations -Woverloaded-virtual -Wno-deprecated -Wdelete-non-virtual-dtor -Werror -Wno-vla 

INCLUDE_FLAGS=-I$(INCLUDE_DIR) -I$(BUILD_DIR)/gen_headers/

ifeq ($(RECOVER_VAR_NAMES),1)
LINKER_FLAGS+=-L$(DEPS_DIR)/libelfin/dwarf/ -L$(DEPS_DIR)/libelfin/elf -lunwind -l:libelf++.a -l:libdwarf++.a
CFLAGS+=-DRECOVER_VAR_NAMES
INCLUDE_FLAGS+=-I$(DEPS_DIR)/libelfin/dwarf -I$(DEPS_DIR)/libelfin/elf/
else
# libelfin has some code that doesn't compile with pedantic
CFLAGS+=-pedantic-errors
endif

LINKER_FLAGS+=-L$(BUILD_DIR)/ -ldl



BUILDER_SRC=$(wildcard $(SRC_DIR)/builder/*.cpp)
BLOCKS_SRC=$(wildcard $(SRC_DIR)/blocks/*.cpp)
UTIL_SRC=$(wildcard $(SRC_DIR)/util/*.cpp)

BUILDER_OBJS=$(subst $(SRC_DIR),$(BUILD_DIR),$(BUILDER_SRC:.cpp=.o))
BLOCKS_OBJS=$(subst $(SRC_DIR),$(BUILD_DIR),$(BLOCKS_SRC:.cpp=.o))
UTIL_OBJS=$(subst $(SRC_DIR),$(BUILD_DIR),$(UTIL_SRC:.cpp=.o))

LIBRARY_OBJS=$(BUILDER_OBJS) $(BLOCKS_OBJS) $(UTIL_OBJS) 
LIBRARY=$(BUILD_DIR)/libbuildit.a

all: executables

.PRECIOUS: $(BUILD_DIR)/builder/%.o 
.PRECIOUS: $(BUILD_DIR)/blocks/%.o 
.PRECIOUS: $(BUILD_DIR)/samples/%.o 
.PRECIOUS: $(BUILD_DIR)/util/%.o

$(BUILD_DIR)/gen_headers/gen/compiler_headers.h:
	echo "#pragma once" > $@
	echo "#define GEN_TEMPLATE_NAME \"$(BASE_DIR)/scratch/code_XXXXXX\"" >> $@
	echo "#define COMPILER_PATH \"$(CC)\"" >> $@


$(BUILD_DIR)/builder/%.o: $(SRC_DIR)/builder/%.cpp $(INCLUDES)
	$(CXX) $(CFLAGS) $< -o $@ $(INCLUDE_FLAGS) -c
$(BUILD_DIR)/blocks/%.o: $(SRC_DIR)/blocks/%.cpp $(INCLUDES)
	$(CXX) $(CFLAGS) $< -o $@ $(INCLUDE_FLAGS) -c
$(BUILD_DIR)/util/%.o: $(SRC_DIR)/util/%.cpp $(INCLUDES)
	$(CXX) $(CFLAGS) $< -o $@ $(INCLUDE_FLAGS) -c


$(BUILD_DIR)/samples/%.o: $(SAMPLES_DIR)/%.cpp $(INCLUDES)
	$(CXX) $(CFLAGS) $< -o $@ $(INCLUDE_FLAGS) -c 


$(LIBRARY): $(LIBRARY_OBJS)
	ar rv $(LIBRARY) $(LIBRARY_OBJS)


$(BUILD_DIR)/sample%: $(BUILD_DIR)/samples/sample%.o $(LIBRARY)
	$(CXX) -o $@ $< $(LINKER_FLAGS)


.PHONY: executables
executables: $(SAMPLES)

SORTED_SAMPLES=$(shell echo $(SAMPLES) | tr " " "\n" | sort -V | tr "\n" " ")

ifeq ($(RECOVER_VAR_NAMES),1)
OUTPUT_DIR=outputs.var_names
else
OUTPUT_DIR=outputs
endif


TEST ?=
run: SHELL:=/bin/bash
run: $(SAMPLES)
	@ if [ "$(TEST)" == "" ]; then \
		total=0;\
                success=0;\
		progress="";\
		fail="";\
		for sample in $(SORTED_SAMPLES); do \
			sample_name=$$(basename $$sample); \
			if [[ $$(head -n1 $(SAMPLES_DIR)/$$sample_name".cpp") != "/*NO_TEST*/" ]]; then \
				((total=total+1)); \
				if cmp -s $(SAMPLES_DIR)/$(OUTPUT_DIR)/$$sample_name <($$sample); then \
					echo -e "\e[32m"$$sample_name: "OK\e[39m"; \
					((success=success+1)); \
					progress=$$progress"\e[32m#\e[39m"; \
				else \
					echo -e "\e[31m"$$sample_name: "FAIL\e[39m"; \
					fail=$$fail"\e[31mX\e[39m"; \
				fi; \
			fi; \
		done; \
		echo -e "["$$progress$$fail"] "$$success/$$total; \
		if [[ $$total == $$success ]]; then \
			exit 0; \
		else \
			exit 1; \
		fi \
	else \
		diff $(SAMPLES_DIR)/$(OUTPUT_DIR)/$(TEST) <($(BUILD_DIR)/$(TEST)) || exit 1;	\
		echo $(TEST): OK; \
	fi

clean:
	- rm -rf $(BUILD_DIR)
clean_scratch:
	- rm -rf $(BASE_DIR)/scratch
