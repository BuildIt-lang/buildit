# Make helper to run the tests
# This is specific to BuildIt

SORTED_SAMPLES=$(shell echo $(SAMPLES) | tr " " "\n" | sort -V | tr "\n" " ")

ifeq ($(RECOVER_VAR_NAMES),1)
OUTPUT_DIR=outputs.var_names
else
OUTPUT_DIR=outputs
endif

TEST ?=
run: SHELL:=/bin/bash
run: $(SAMPLES)
	@ export CLICOLOR=1
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
					echo -e "\033[32m"$$sample_name: "OK\033[39m"; \
					((success=success+1)); \
					progress=$$progress"\033[32m#\033[39m"; \
				else \
					echo -e "\033[31m"$$sample_name: "FAIL\033[39m"; \
					fail=$$fail"\033[31mX\033[39m"; \
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

