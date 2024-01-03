ifeq ($(MAKECMDGOALS), format)
CHECK_CONFIG=0
endif

format:
	clang-format -i -style=file $$(find \( -name "*.h" -o -name "*.cpp" \) -a -not -path "./deps/*")
