# Initialize config parameters if not initialized
RECOVER_VAR_NAMES ?= 0
TRACER_USE_LIBUNWIND ?= 0
DEBUG ?= 0
ENABLE_D2X ?= 0
EXTRA_CFLAGS?=

ifeq ($(ENABLE_D2X),1)
DEBUG=1
RECOVER_VAR_NAMES=1
endif

ifeq ($(RECOVER_VAR_NAMES),1)
ifneq ($(shell uname), Linux)
$(error RECOVER_VAR_NAMES only supported on Linux)
endif
DEBUG=1
endif



DEPS_LIST=
LINKER_FLAGS=

ifeq ($(ENABLE_D2X),1)
LINKER_FLAGS+=-Wl,--start-group -L $(D2X_BUILD_DIR) -ld2x 
DEPS_LIST+=$(BUILD_DIR)/d2x.dep
endif

# Create CFLAGS, LINKER_FLAGS, CFLAGS_INTERNAL and INCLUDE_FLAGS based on config
CFLAGS_INTERNAL=-std=c++11 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wmissing-declarations 
CFLAGS_INTERNAL+=-Woverloaded-virtual -Wno-deprecated -Wdelete-non-virtual-dtor -Werror -Wno-vla -pedantic-errors 
CFLAGS=
LINKER_FLAGS+=-L$(BUILD_DIR)/ -l$(LIBRARY_NAME)
INCLUDE_FLAGS=-I$(INCLUDE_DIR) -I$(BUILD_DIR)/gen_headers/

ifeq ($(ENABLE_D2X),1)
LINKER_FLAGS+=-Wl,--end-group
CFLAGS+=-DENABLE_D2X
INCLUDE_FLAGS+=-I $(D2X_DIR)/include
endif

ifeq ($(DEBUG),1)
CFLAGS+=-g -gdwarf-4
LINKER_FLAGS+=-g -gdwarf-4
else
CFLAGS_INTERNAL+=-O3
endif



ifeq ($(TRACER_USE_LIBUNWIND),1)
CFLAGS_INTERNAL+=-DTRACER_USE_LIBUNWIND
endif


LIBUNWIND_PATH ?= _UNSET_
ifeq ($(RECOVER_VAR_NAMES),1)
LINKER_FLAGS+=-ldwarf -lunwind
CFLAGS_INTERNAL+=-DRECOVER_VAR_NAMES
ifneq ($(LIBUNWIND_PATH),_UNSET_)
INCLUDE_FLAGS+=-I $(LIBUNWIND_PATH)/include
LINKER_FLAGS+=-L $(LIBUNWIND_PATH)/lib
endif
else
ifeq ($(TRACER_USE_LIBUNWIND),1)
LINKER_FLAGS+=-lunwind
ifneq ($(LIBUNWIND_PATH),_UNSET_)
INCLUDE_FLAGS+=-I $(LIBUNWIND_PATH)/include
LINKER_FLAGS+=-L $(LIBUNWIND_PATH)/lib
endif
endif
endif


LINKER_FLAGS+=-ldl
CFLAGS+=$(EXTRA_CFLAGS)
# --- flags are all ready
