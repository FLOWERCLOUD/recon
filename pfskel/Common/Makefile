COMPILER = g++
COMPILE_OPTIONS	= -D_DEBUG -g
ROOT_DIR = ..

driver: driver.cpp common.o skeleton.o 
	$(COMPILER) driver.cpp common.o	skeleton.o -o driver $(COMPILE_OPTIONS)

common.o skeleton.o : dynamicArray.h

common.o : common.h common.cpp stack.h
	$(COMPILER) -c -o common.o common.cpp $(COMPILE_OPTIONS)

skeleton.o: skeleton.h skeleton.cpp common.o
	$(COMPILER) -c -o skeleton.o skeleton.cpp $(COMPILE_OPTIONS)


%: 
	@$(MAKE) -f $(ROOT_DIR)/default.mk $@
