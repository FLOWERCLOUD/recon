COMMON_DIR = ../Common
ROOT_DIR = ..

OBJS = expandVol.o driver.o
COMMON_OBJS = $(COMMON_DIR)/common.o

include $(ROOT_DIR)/defvars.mk

driver: $(OBJS)
	$(COMPILER) $(EXECOUTPUT) driver $(OBJS) $(COMMON_OBJS) $(COMPILE_OPTIONS) -I$(COMMON_DIR)

expandVol.o driver.o : common

expandVol.o : expandVol.h expandVol.cpp
	$(COMPILER) $(OUTPUT) expandVol.o $(COMPILE) expandVol.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR)

driver.o : driver.cpp
	$(COMPILER) $(OUTPUT) driver.o $(COMPILE) driver.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR)


common:
	$(MAKE) -C $(COMMON_DIR) common.o

cleanall: clean
	$(MAKE) -C $(COMMON_DIR) clean

%: 
	@$(MAKE) -f $(ROOT_DIR)/default.mk $@

