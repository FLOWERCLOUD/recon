COMMON_DIR = ../Common

ROOT_DIR = ..

OBJS = streamLn.o driver.o
COMMON_OBJS = $(COMMON_DIR)/common.o $(COMMON_DIR)/skeleton.o

CRITPTS_DIR = ../CritPts
CRITPTS_OBJS = $(CRITPTS_DIR)/critPts.o
JAMA_DIR = ../JAMA
TNT_DIR = ../TNT

HD_DIR = ../HighDiverg
HD_OBJS = $(HD_DIR)/highDiverg.o $(HD_DIR)/localMinDiv.o $(HD_DIR)/allDiv.o

DISTFIELD_DIR = ../DistField
DISTFIELD_OBJS = $(DISTFIELD_DIR)/getDT.o

include $(ROOT_DIR)/defvars.mk

driver: $(OBJS) critPts common skeleton highDiverg distField
	$(COMPILER) $(EXECOUTPUT) driver  $(COMMON_OBJS) $(OBJS) $(CRITPTS_OBJS) $(HD_OBJS) $(DISTFIELD_OBJS) $(COMPILE_OPTIONS) -I$(COMMON_DIR) 

streamLn.o driver.o : $(COMMON_DIR)/common.h $(COMMON_DIR)/skeleton.h

streamLn.o : streamLn.h streamLn.cpp
	$(COMPILER) $(OUTPUT) streamLn.o $(COMPILE) streamLn.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR) -I$(CRITPTS_DIR) -I$(JAMA_DIR) -I$(TNT_DIR)

driver.o : driver.cpp
	$(COMPILER) $(OUTPUT) driver.o $(COMPILE) driver.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR) -I$(CRITPTS_DIR) -I$(JAMA_DIR) -I$(TNT_DIR) -I$(HD_DIR) -I$(DISTFIELD_DIR)


critPts:
	$(MAKE) -C $(CRITPTS_DIR) critPts.o

common:
	$(MAKE) -C $(COMMON_DIR) common.o

skeleton:
	$(MAKE) -C $(COMMON_DIR) skeleton.o

highDiverg:
	$(MAKE) -C $(HD_DIR) highDiverg.o

distField: 
	$(MAKE) -C $(DISTFIELD_DIR) getDT.o

cleanall: clean
	$(MAKE) -C $(COMMON_DIR) clean

%: 
	@$(MAKE) -f $(ROOT_DIR)/default.mk $@


