COMMON_DIR = ../Common
ROOT_DIR = ..

# OBJS = potVect.o driver.o
OBJS = driver.o connectivity.o
COMMON_OBJS = $(COMMON_DIR)/common.o

include $(ROOT_DIR)/defvars.mk

driver: $(OBJS)
	$(COMPILER) $(EXECOUTPUT) driver $(OBJS) $(COMMON_OBJS) $(COMPILE_OPTIONS) -I$(COMMON_DIR)

driver.o connectivity.o : common

driver.o : driver.cpp
	$(COMPILER) $(OUTPUT) driver.o $(COMPILE) driver.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR)

connectivity.o : connectivity.cpp	
	$(COMPILER) $(OUTPUT) connectivity.o $(COMPILE) connectivity.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR)

common:
	$(MAKE) -C $(COMMON_DIR)

cleanall: clean
	$(MAKE) -C $(COMMON_DIR) clean

%: 
	@$(MAKE) -f $(ROOT_DIR)/default.mk $@

