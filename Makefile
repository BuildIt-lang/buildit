-include Makefile.inc
LIBRARY_NAME=buildit

include make/dirs.mk
include make/setvars.mk
include make/verbose.mk

# Default target should be at the top
all: executables

CHECK_CONFIG=1
CONFIG_STR=DEBUG=$(DEBUG) RECOVER_VAR_NAMES=$(RECOVER_VAR_NAMES) TRACER_USE_LIBUNWIND=$(TRACER_USE_LIBUNWIND)
CONFIG_STR+=EXTRA_CFLAGS=$(EXTRA_CFLAGS) ENABLE_D2X=$(ENABLE_D2X)


# Create a scratch directory where the files are stored
$(shell mkdir -p $(BASE_DIR)/scratch)

include make/pkgconfig.mk
include make/format.mk
include make/stable_config.mk
include make/deps.mk
include make/buildit_rules.mk
include make/tests.mk

clean:
	- rm -rf $(BUILD_DIR)
clean_scratch:
	- rm -rf $(BASE_DIR)/scratch

