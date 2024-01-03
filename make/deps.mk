# Build dependencies as required

# D2X
# this d2x.dep is a representative of everything 
# that d2x provides 
$(BUILD_DIR)/d2x.dep: $(D2X_DEPS) $(INCLUDES)
	$(MAKE) -C $(D2X_DIR) BUILDIT_DIR=$(BASE_DIR) lib
	touch $(BUILD_DIR)/d2x.dep
