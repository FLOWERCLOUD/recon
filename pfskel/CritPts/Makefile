COMMON_DIR = ../Common
ROOT_DIR = ..
TNT_HOME = ../TNT
JAMA_HOME = ../JAMA

OBJS = critPts.o
COMMON_OBJS = $(COMMON_DIR)/common.o

include $(ROOT_DIR)/defvars.mk


driver: $(OBJS) driver.o
	$(COMPILER) $(EXECOUTPUT) driver driver.o $(OBJS) $(COMMON_OBJS) $(COMPILE_OPTIONS) -I$(COMMON_DIR) -I$(TNT_HOME) -I$(JAMA_HOME)


driver.o critPts.o: common 

critPts.o: critPts.cpp critPts.h
	$(COMPILER) $(OUTPUT) critPts.o $(COMPILE) critPts.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR) -I$(TNT_HOME) -I$(JAMA_HOME)

driver.o : driver.cpp
	$(COMPILER) $(OUTPUT) driver.o $(COMPILE) driver.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR) -I$(TNT_HOME) -I$(JAMA_HOME)


common:
	$(MAKE) -C $(COMMON_DIR) common.o

cleanall: clean
	$(MAKE) -C $(COMMON_DIR) clean

%: 
	@$(MAKE) -f $(ROOT_DIR)/default.mk $@
