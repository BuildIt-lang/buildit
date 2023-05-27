# This make helper checks if the CONFIG_STR has changed since the last invokation and 
# cleans the build directory if it has

ifeq ($(CHECK_CONFIG), 1)
CONFIG_FILE=$(BUILD_DIR)/build.config
$(shell mkdir -p $(BUILD_DIR))
$(shell touch $(CONFIG_FILE))

ifneq ($(shell cat $(CONFIG_FILE)), $(CONFIG_STR))
$(warning Previous config and current config does not match! Rebuilding)
$(shell rm -rf $(BUILD_DIR))
$(shell mkdir -p $(BUILD_DIR))
$(shell echo $(CONFIG_STR) > $(CONFIG_FILE))
endif
endif
