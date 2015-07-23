COMMON_DIR = ../Common
CRITPTS_DIR = ../CritPts
JAMA_DIR = ../JAMA
TNT_DIR = ../TNT
ROOT_DIR = ..

OBJS = gradField.o driver.o
COMMON_OBJS = $(COMMON_DIR)/common.o
CRITPTS_OBJS = $(CRITPTS_DIR)/critPts.o

include $(ROOT_DIR)/defvars.mk

driver: $(OBJS)
	$(COMPILER) $(EXECOUTPUT) driver $(OBJS) $(COMMON_OBJS) $(CRITPTS_OBJS) $(COMPILE_OPTIONS) -I$(COMMON_DIR)

gradField.o driver.o : common

gradField.o : gradField.h gradField.cpp critPts
	$(COMPILER) $(OUTPUT) gradField.o $(COMPILE) gradField.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR) -I$(CRITPTS_DIR) -I$(TNT_DIR) -I$(JAMA_DIR)

driver.o : driver.cpp
	$(COMPILER) $(OUTPUT) driver.o $(COMPILE) driver.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR)


common:
	$(MAKE) -C $(COMMON_DIR) common.o

critPts:
	$(MAKE) -C $(CRITPTS_DIR) critPts.o

cleanall: clean
	$(MAKE) -C $(COMMON_DIR) clean
	$(MAKE) -C $(CRITPTS_DIR) clean

%: 
	@$(MAKE) -f $(ROOT_DIR)/default.mk $@

