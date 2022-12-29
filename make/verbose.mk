# This make helper changes the actual compilation commands
# based on the verbosity level. The input is basically the variable V

# Default verbosity level is 0
V?=0
ifeq ($V,0) 
CXXV  =@echo CXX"    "$(notdir $@); $(CXX)
CXXLDV=@echo CXXLD"  "$(notdir $@); $(CXX)
ARV   =@echo AR"     "$(notdir $@); $(AR)
else
CXXV=$(CXX)
CXXLDV=$(CXX)
ARV=$(AR)
endif
