CC = gcc
CPP = g++
#put all your object files here
OBJS = sp_complete_unit_test.o SPImageProc.o SPPoint.o SPConfig.o SPLogger.o main_aux.o SPKDTree.o SPKDArray.o SPBPriorityQueue.o 
#The executabel filename
EXEC = sp_complete_unit_test
TESTS_DIR = ./unit_tests
INCLUDEPATH=/usr/local/lib/opencv-3.1.0/include/
LIBPATH=/usr/local/lib/opencv-3.1.0/lib/
LIBS=-lopencv_xfeatures2d -lopencv_features2d \
-lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_core


CPP_COMP_FLAG = -std=c++11 -Wall -Wextra \
-Werror -pedantic-errors -DNDEBUG

C_COMP_FLAG = -std=c99 -Wall -Wextra \
-Werror -pedantic-errors -DNDEBUG

$(EXEC): $(OBJS)
	$(CPP) $(OBJS) -L$(LIBPATH) $(LIBS) -o $@
sp_complete_unit_test.o: $(TESTS_DIR)/sp_complete_unit_test.cpp $(TESTS_DIR)/unit_test_util.h #put dependencies here!
	$(CPP) $(CPP_COMP_FLAG) -I$(INCLUDEPATH) -c $(TESTS_DIR)/$*.cpp
main_aux.o: main_aux.cpp
	$(CPP) $(CPP_COMP_FLAG) -I$(INCLUDEPATH) -c $*.cpp
#a rule for building a simple c++ source file
#use g++ -MM SPImageProc.cpp to see dependencies
SPImageProc.o: SPImageProc.cpp SPImageProc.h SPConfig.h SPPoint.h SPLogger.h
	$(CPP) $(CPP_COMP_FLAG) -I$(INCLUDEPATH) -c $*.cpp

#a rule for building a simple c source file
#use "gcc -MM SPPoint.c" to see the dependencies
SPPoint.o: SPPoint.c SPPoint.h 
	$(CC) $(C_COMP_FLAG) -c $*.c
SPConfig.o: SPConfig.c SPConfig.h 
	$(CC) $(C_COMP_FLAG) -c $*.c
SPLogger.o: SPLogger.c SPLogger.h 
	$(CC) $(C_COMP_FLAG) -c $*.c
SPBPriorityQueue.o: SPBPriorityQueue.c SPBPriorityQueue.h 
	$(CC) $(C_COMP_FLAG) -c $*.c
SPKDArray.o: SPKDArray.c SPKDArray.h 
	$(CC) $(C_COMP_FLAG) -c $*.c
SPKDTree.o: SPKDTree.c SPKDTree.h SPKDArray.h SPBPriorityQueue.h 
	$(CC) $(C_COMP_FLAG) -c $*.c