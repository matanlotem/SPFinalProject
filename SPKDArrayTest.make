CC = gcc
OBJS = smallKDArrayTester.o SPKDArray.o SPPoint.o SPLogger.o
EXEC = smallKDArrayTester
TESTS_DIR = ./unit_tests
COMP_FLAG = -std=c99 -Wall -Wextra \
-Werror -pedantic-errors

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@
smallKDArrayTester.o: $(TESTS_DIR)/smallKDArrayTester.c $(TESTS_DIR)/unit_test_util.h SPKDArray.h
	$(CC) $(COMP_FLAG) -c $(TESTS_DIR)/$*.c
SPKDArray.o: SPKDArray.c SPKDArray.h 
	$(CC) $(COMP_FLAG) -c $*.c
SPPoint.o: SPPoint.c SPPoint.h 
	$(CC) $(COMP_FLAG) -c $*.c
SPLogger.o: SPLogger.c SPLogger.h 
	$(CC) $(COMP_FLAG) -c $*.c

clean:
	rm -f $(OBJS) $(EXEC)
