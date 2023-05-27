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

