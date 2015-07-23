COMMON_DIR = ../Common
CRITPTS_DIR = ../CritPts
GRADFIELD_DIR = ../GradField
JAMA_DIR = ../JAMA
TNT_DIR = ../TNT
ROOT_DIR = ..

OBJS = adaptiveCP.o
COMMON_OBJS = $(COMMON_DIR)/common.o
CRITPTS_OBJS = $(CRITPTS_DIR)/critPts.o
GRADFIELD_OBJS = $(GRADFIELD_DIR)/gradField.o

include $(ROOT_DIR)/defvars.mk

adaptiveCP: $(OBJS) common critPts gradField
	$(COMPILER) $(EXECOUTPUT) adaptiveCP $(OBJS) $(COMMON_OBJS) $(CRITPTS_OBJS) $(GRADFIELD_OBJS) $(COMPILE_OPTIONS) -I$(COMMON_DIR)


adaptiveCP.o : $(COMMON_DIR)/common.h $(CRITPTS_DIR)/critPts.h $(GRADFIELD_DIR)/gradField.h adaptiveCP.cpp
	$(COMPILER) $(OUTPUT) adaptiveCP.o $(COMPILE) adaptiveCP.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR) -I$(CRITPTS_DIR) -I$(TNT_DIR) -I$(JAMA_DIR) -I$(GRADFIELD_DIR)


common:
	$(MAKE) -C $(COMMON_DIR) common.o

critPts:
	$(MAKE) -C $(CRITPTS_DIR) critPts.o

gradField:
	$(MAKE) -C $(GRADFIELD_DIR) gradField.o

cleanall: clean
	$(MAKE) -C $(COMMON_DIR) clean
	$(MAKE) -C $(CRITPTS_DIR) clean
	$(MAKE) -C $(GRADFIELD_DIR) clean

%: 
	@$(MAKE) -f $(ROOT_DIR)/default.mk $@

