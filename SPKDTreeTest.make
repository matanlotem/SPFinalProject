CC = gcc
OBJS = testerKdTree.o SPKDTree.o SPKDArray.o SPPoint.o SPBPriorityQueue.o SPLogger.o
EXEC = testerKdTree
TESTS_DIR = ./unit_tests
COMP_FLAG = -std=c99 -Wall -Wextra \
-Werror -pedantic-errors

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@
testerKdTree.o: $(TESTS_DIR)/testerKdTree.c $(TESTS_DIR)/unit_test_util.h SPKDTree.h SPKDArray.h SPPoint.h
	$(CC) $(COMP_FLAG) -c $(TESTS_DIR)/$*.c
SPKDArray.o: SPKDArray.c SPKDArray.h 
	$(CC) $(COMP_FLAG) -c $*.c
SPPoint.o: SPPoint.c SPPoint.h 
	$(CC) $(COMP_FLAG) -c $*.c
SPKDTree.o: SPKDTree.c SPKDTree.h 
	$(CC) $(COMP_FLAG) -c $*.c
SPBPriorityQueue.o: SPBPriorityQueue.c SPBPriorityQueue.h
	$(CC) $(COMP_FLAG) -c $*.c
SPLogger.o: SPLogger.c SPLogger.h 
	$(CC) $(COMP_FLAG) -c $*.c
	
clean:
	rm -f $(OBJS) $(EXEC)
