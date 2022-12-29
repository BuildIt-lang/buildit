# This make helper provides two new targets compile-flags and linker-flags that
# print out the flags to link against this library
# This requires CFLAGS, INCLUDE_FLAGS and LINKER_FLAGS to be defined

# This module also disables CHECK_CONFIG if these flags are invoked

ifeq ($(MAKECMDGOALS), compile-flags)
CHECK_CONFIG=0
endif
ifeq ($(MAKECMDGOALS), linker-flags)
CHECK_CONFIG=0
endif

.PHONY: compile-flags linker-flags
compile-flags:
	@echo $(CFLAGS) $(INCLUDE_FLAGS)

linker-flags:
	@echo $(LINKER_FLAGS)
