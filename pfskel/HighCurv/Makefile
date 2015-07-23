COMMON_DIR = ../Common
ROOT_DIR = ..

OBJS = localMinDiv.o allDiv.o highDiverg.o driver.o
COMMON_OBJS = $(COMMON_DIR)/common.o

include $(ROOT_DIR)/defvars.mk

driver: $(OBJS) common
	$(COMPILER) $(EXECOUTPUT) driver $(OBJS) $(COMMON_OBJS) $(COMPILE_OPTIONS) -I$(COMMON_DIR)

localMinDiv.o allDiv.o highDiverg.o : $(COMMON_DIR)/common.h

localMinDiv.o : localMinDiv.h localMinDiv.cpp
	$(COMPILER) $(OUTPUT) localMinDiv.o $(COMPILE) localMinDiv.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR) 

allDiv.o : allDiv.h allDiv.cpp
	$(COMPILER) $(OUTPUT) allDiv.o $(COMPILE) allDiv.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR) 

highDiverg.o : highDiverg.h highDiverg.cpp localMinDiv.h allDiv.h
	$(COMPILER) $(OUTPUT) highDiverg.o $(COMPILE) highDiverg.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR) 

driver.o : driver.cpp
	$(COMPILER) $(OUTPUT) driver.o $(COMPILE) driver.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR)


common:
	$(MAKE) -C $(COMMON_DIR)

cleanall: clean
	$(MAKE) -C $(COMMON_DIR) clean

%: 
	@$(MAKE) -f $(ROOT_DIR)/default.mk $@


