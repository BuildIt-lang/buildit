BASE_DIR=$(shell pwd)
SRC_DIR=$(BASE_DIR)/src
BUILD_DIR=$(BASE_DIR)/build
INCLUDE_DIR=$(BASE_DIR)/include


INCLUDES=$(wildcard $(INCLUDE_DIR)/*.h)

$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(BUILD_DIR)/blocks)
$(shell mkdir -p $(BUILD_DIR)/builder)

all: $(BUILD_DIR)/blocks.a $(BUILD_DIR)/builder.a

$(BUILD_DIR)/builder.a: $(BUILD_DIR)/builder/builder_context.o
	ar rcs $@ $^


$(BUILD_DIR)/blocks.a: 
	ar rcs $@ $^

$(BUILD_DIR)/builder/builder_context.o: $(SRC_DIR)/builder/builder_context.cpp
	$(CXX) $(CFLAGS) $< -o $@ -I$(INCLUDE_DIR) -c


clean:
	- rm -rf build
