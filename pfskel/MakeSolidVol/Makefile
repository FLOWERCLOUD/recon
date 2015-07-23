
COMMON_DIR = ../Common
ROOT_DIR = ..

OBJS = makeSolidVol.o driver.o
COMMON_OBJS = $(COMMON_DIR)/common.o

include $(ROOT_DIR)/defvars.mk

driver: $(OBJS) common
	$(COMPILER) $(EXECOUTPUT) driver $(OBJS) $(COMMON_OBJS) $(COMPILE_OPTIONS) -I$(COMMON_DIR)


makeSolidVol.o : makeSolidVol.h makeSolidVol.cpp
	$(COMPILER) $(OUTPUT) makeSolidVol.o $(COMPILE) makeSolidVol.cpp $(COMPILE_OPTIONS)

driver.o : driver.cpp
	$(COMPILER) $(OUTPUT) driver.o $(COMPILE) driver.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR)

common:
	$(MAKE) -C $(COMMON_DIR)

cleanall: clean
	$(MAKE) -C $(COMMON_DIR) clean

%: 
	@$(MAKE) -f $(ROOT_DIR)/default.mk $@


