# Actual rules for building the library and the executables
.PRECIOUS: $(BUILD_DIR)/%.o 
.PRECIOUS: $(BUILD_DIR)/samples/%.o 

$(BUILD_DIR)/gen_headers/gen/compiler_headers.h:
	@mkdir -p $(@D)
	@echo "#pragma once" > $@
	@echo "#define GEN_TEMPLATE_NAME \"$(BASE_DIR)/scratch/code_XXXXXX\"" >> $@
	@echo "#define COMPILER_PATH \"$(CC)\"" >> $@
	@echo "#define CXX_COMPILER_PATH \"$(CXX)\"" >> $@
$(LIBRARY_OBJS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDES) $(DEPS_LIST)
	@mkdir -p $(@D)
	$(CXXV) $(CFLAGS_INTERNAL) $(CFLAGS) $< -o $@ $(INCLUDE_FLAGS) -c
$(BUILD_DIR)/samples/%.o: $(SAMPLES_DIR)/%.cpp $(INCLUDES) $(DEPS_LIST)
	@mkdir -p $(@D)
	$(CXXV) $(CFLAGS_INTERNAL) $(CFLAGS) $< -o $@ $(INCLUDE_FLAGS) -c
$(LIBRARY): $(LIBRARY_OBJS) $(DEPS_LIST)
	@mkdir -p $(@D)
	$(ARV) cr $(LIBRARY) $(LIBRARY_OBJS)
$(BUILD_DIR)/sample%: $(BUILD_DIR)/samples/sample%.o $(LIBRARY) $(DEPS_LIST)
	@mkdir -p $(@D)
	$(CXXLDV) -o $@ $< $(LINKER_FLAGS)

.PHONY: executables
executables: $(SAMPLES)
