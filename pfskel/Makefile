COMMON_DIR = Common
ROOT_DIR = .
potField_DIR	 =	PotField
gradField_DIR	 =	GradField
hCB_DIR		 = 	HCB
streamLn_DIR	 =	StreamLn
critPts_DIR	 =	CritPts
expandVol_DIR	 =	ExpandVol
makeSolidVol_DIR = 	MakeSolidVol
highDiverg_DIR	 = 	HighDiverg
padVol_DIR	 = 	PadVol
distField_DIR    =      DistField
pfSkel_DIR	 =	.

OBJS = driver.o pfSkel.o \
	$(potField_DIR)/potVect.o \
	$(gradField_DIR)/gradField.o \
	$(streamLn_DIR)/streamLn.o \
	$(critPts_DIR)/critPts.o \
	$(expandVol_DIR)/expandVol.o \
	$(makeSolidVol_DIR)/makeSolidVol.o \
	$(highDiverg_DIR)/highDiverg.o $(highDiverg_DIR)/localMinDiv.o \
	$(highDiverg_DIR)/allDiv.o \
	$(distField_DIR)/getDT.o

COMMON_OBJS = $(COMMON_DIR)/common.o $(COMMON_DIR)/skeleton.o

include $(ROOT_DIR)/defvars.mk



TNT_HOME = TNT
JAMA_HOME = JAMA


pfSkel: driver.o pfSkel.o common potField gradField streamLn critPts \
        expandVol makeSolidVol highDiverg distField
	$(COMPILER) $(OBJS) $(COMMON_OBJS) $(COMPILE_OPTIONS) $(OUTPUT) \
	pfSkel -lm

pfSkel.o driver.o : $(COMMON_DIR)/common.h

pfSkel.o : $(pfSkel_DIR)/pfSkel.h $(pfSkel_DIR)/pfSkel.cpp
	$(COMPILER) $(OUTPUT) pfSkel.o $(COMPILE) $(pfSkel_DIR)/pfSkel.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR) -I$(TNT_HOME) -I$(JAMA_HOME)

driver.o : driver.cpp
	$(COMPILER) $(OUTPUT) driver.o $(COMPILE) driver.cpp $(COMPILE_OPTIONS) -I$(COMMON_DIR) -I$(TNT_HOME) -I$(JAMA_HOME)


common:
	$(MAKE) -C $(COMMON_DIR) common.o

potField:
	$(MAKE) -C $(potField_DIR) potVect.o

gradField:
	$(MAKE) -C $(gradField_DIR) gradField.o

critPts:
	$(MAKE) -C $(critPts_DIR) critPts.o

highDiverg:
	$(MAKE) -C $(highDiverg_DIR) highDiverg.o allDiv.o localMinDiv.o

streamLn:
	$(MAKE) -C $(streamLn_DIR) streamLn.o

expandVol:
	$(MAKE) -C $(expandVol_DIR) expandVol.o

makeSolidVol:
	$(MAKE) -C $(makeSolidVol_DIR) makeSolidVol.o

padVol:
	$(MAKE) -C $(padVol_DIR) driver

distField:
	$(MAKE) -C $(distField_DIR) getDT.o

all:
# Note: leave out the symlinks on Cygwin. Since windows does not make the distinction between upper/lower case, we can't create them using the names of the directories.
	$(MAKE) -C $(potField_DIR) driver
	#rm -f potField
	#ln -s $(potField_DIR)/driver potField
	$(MAKE) -C $(gradField_DIR) driver
	#rm -f gradField
	#ln -s $(gradField_DIR)/driver gradField
	$(MAKE) -C $(critPts_DIR) driver
	#rm -f critPts
	#ln -s $(critPts_DIR)/driver critPts
	$(MAKE) -C $(highDiverg_DIR) driver
	#rm -f highDiverg
	#ln -s $(highDiverg_DIR)/driver highDiverg
	$(MAKE) -C $(streamLn_DIR) driver
	#rm -f streamLn
	#ln -s $(streamLn_DIR)/driver streamLn
	$(MAKE) -C $(expandVol_DIR) driver
	#rm -f expandVol
	#ln -s $(expandVol_DIR)/driver expandVol
	$(MAKE) -C $(makeSolidVol_DIR) driver
	#rm -f makeSolidVol
	#ln -s $(makeSolidVol_DIR)/driver makeSolidVol
	$(MAKE) -C $(padVol_DIR) driver
	#rm -f padVol
	#ln -s $(padVol_DIR)/driver padVol
	$(MAKE) -C $(padVol_DIR) driver
	#rm -f distField
	$(MAKE) -C $(distField_DIR) driver
	#ln -s $(distField_DIR)/driver distField
	$(MAKE) pfSkel


cleanall: clean
	$(MAKE) -C $(COMMON_DIR) clean
	$(MAKE) -C $(potField_DIR) clean
	$(MAKE) -C $(gradField_DIR) clean
	$(MAKE) -C $(critPts_DIR) clean
	$(MAKE) -C $(highDiverg_DIR) clean
	$(MAKE) -C $(streamLn_DIR) clean
	$(MAKE) -C $(expandVol_DIR) clean
	$(MAKE) -C $(makeSolidVol_DIR) clean
	$(MAKE) -C $(padVol_DIR) clean
	$(MAKE) -C $(distField_DIR) clean
	#rm -f potField gradField critPts highDiverg streamLn expandVol \
	#	makeSolidVol padVol distField

%: 
	@$(MAKE) -f $(ROOT_DIR)/default.mk $@
