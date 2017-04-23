#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "unit_test_util.h" //SUPPORTING MACROS ASSERT_TRUE/ASSERT_FALSE etc..
#include "../SPConsts.h"
#include "../SPLogger.h"

#define DEV_NULL "/dev/null"
#define LOGGER_TEST_DIR "unit_tests/sp_logger/"

// This is a helper function which checks if two files are identical
static bool identicalFiles(const char* fname1, const char* fname2) {
	FILE *fp1, *fp2;
	fp1 = fopen(fname1, "r");
	fp2 = fopen(fname2, "r");
	char ch1 = EOF, ch2 = EOF;

	if (fp1 == NULL) {
		return false;
	} else if (fp2 == NULL) {
		fclose(fp1);
		return false;
	} else {
		ch1 = getc(fp1);
		ch2 = getc(fp2);

		while ((ch1 != EOF) && (ch2 != EOF) && (ch1 == ch2)) {
			ch1 = getc(fp1);
			ch2 = getc(fp2);
		}
		fclose(fp1);
		fclose(fp2);
	}
	if (ch1 == ch2) {
		return true;
	} else {
		return false;
	}
}

//Logger is not defined
static bool basicLoggerTest() {

	// write to work with undefined logger
	ASSERT_TRUE(spLoggerPrintError("A","sp_logger_unit_test.c",__func__,__LINE__) == SP_LOGGER_UNDEFINED);
	spLoggerDestroy();

	// try to define logger twice
	ASSERT_TRUE(spLoggerCreate(DEV_NULL,SP_LOGGER_DEBUG_INFO_WARNING_ERROR_LEVEL) == SP_LOGGER_SUCCESS);
	ASSERT_TRUE(spLoggerCreate(DEV_NULL,SP_LOGGER_DEBUG_INFO_WARNING_ERROR_LEVEL) == SP_LOGGER_DEFINED);
	spLoggerDestroy();
	ASSERT_TRUE(spLoggerCreate(DEV_NULL,SP_LOGGER_DEBUG_INFO_WARNING_ERROR_LEVEL) == SP_LOGGER_SUCCESS);
	spLoggerDestroy();

	// create logger to file without permissions
	// commented out because will not work in test machine
	//ASSERT_TRUE(spLoggerCreate(LOGGER_TEST_DIR "readOnly.log",SP_LOGGER_DEBUG_INFO_WARNING_ERROR_LEVEL) == SP_LOGGER_CANNOT_OPEN_FILE);
	//spLoggerDestroy();

	return true;
}

// Create Logger and for given logger level and write log messages at different levels
	static bool basicLoggerLevelTest(const char* lvlStr, SP_LOGGER_LEVEL lvl) {
	char expectedFile[STR_LEN], testFile[STR_LEN];
	sprintf(expectedFile,"%sbasicLogger%sTestExp.log",LOGGER_TEST_DIR, lvlStr);
	sprintf(testFile,"%sbasicLogger%sTest.log",LOGGER_TEST_DIR, lvlStr);

	// create logger
	spLoggerDestroy();
	ASSERT_TRUE(spLoggerCreate(testFile,lvl) == SP_LOGGER_SUCCESS);

	// print to logger
	ASSERT_TRUE(spLoggerPrintError("MSGA","sp_logger_unit_test.c",__func__,1) == SP_LOGGER_SUCCESS);
	ASSERT_TRUE(spLoggerPrintWarning("MSGB","sp_logger_unit_test.c",__func__,2) == SP_LOGGER_SUCCESS);
	ASSERT_TRUE(spLoggerPrintInfo("MSGC") == SP_LOGGER_SUCCESS);
	ASSERT_TRUE(spLoggerPrintDebug("MSGD","sp_logger_unit_test.c",__func__,4) == SP_LOGGER_SUCCESS);
	ASSERT_TRUE(spLoggerPrintMsg("MSGE") == SP_LOGGER_SUCCESS);

	// destroy logger
	spLoggerDestroy();

	// check if expected results
	ASSERT_TRUE(identicalFiles(testFile,expectedFile));

	return true;
}

static bool basicLoggerAllLevelsTest() {
	//Only Errors should be printed
	ASSERT_TRUE(basicLoggerLevelTest("Error", SP_LOGGER_ERROR_LEVEL));
	//Only Errors and Warnings should be printed
	ASSERT_TRUE(basicLoggerLevelTest("Warning", SP_LOGGER_WARNING_ERROR_LEVEL));
	//Only Errors, Warning and Info should be printed
	ASSERT_TRUE(basicLoggerLevelTest("Info", SP_LOGGER_INFO_WARNING_ERROR_LEVEL));
	//All messages should be printed in debug level
	ASSERT_TRUE(basicLoggerLevelTest("Debug", SP_LOGGER_DEBUG_INFO_WARNING_ERROR_LEVEL));

	return true;
}

//Validate logger fails nicely for invalid arguments
static bool nullArgLoggerTest() {
	// undefined
	spLoggerDestroy();
	ASSERT_TRUE(spLoggerPrintError("", "", "", 1) == SP_LOGGER_UNDEFINED);
	ASSERT_TRUE(spLoggerPrintWarning("", "", "", 1) == SP_LOGGER_UNDEFINED);
	ASSERT_TRUE(spLoggerPrintInfo("") == SP_LOGGER_UNDEFINED);
	ASSERT_TRUE(spLoggerPrintDebug("", "", "", 1) == SP_LOGGER_UNDEFINED);
	ASSERT_TRUE(spLoggerPrintMsg("") == SP_LOGGER_UNDEFINED);

	// create logger
	ASSERT_TRUE(spLoggerCreate(DEV_NULL,SP_LOGGER_DEBUG_INFO_WARNING_ERROR_LEVEL) == SP_LOGGER_SUCCESS);

	// error
	ASSERT_TRUE(spLoggerPrintError(NULL, "", "", 1) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintError("", NULL, "", 1) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintError("", "", NULL, 1) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintError("", "", "", -7) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintError("", "", "", 1) == SP_LOGGER_SUCCESS);
	// warning
	ASSERT_TRUE(spLoggerPrintWarning(NULL, "", "", 1) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintWarning("", NULL, "", 1) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintWarning("", "", NULL, 1) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintWarning("", "", "", -7) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintWarning("", "", "", 1) == SP_LOGGER_SUCCESS);
	// info
	ASSERT_TRUE(spLoggerPrintInfo(NULL) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintInfo("") == SP_LOGGER_SUCCESS);
	// debug
	ASSERT_TRUE(spLoggerPrintDebug(NULL, "", "", 1) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintDebug("", NULL, "", 1) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintDebug("", "", NULL, 1) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintDebug("", "", "", -7) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintDebug("", "", "", 1) == SP_LOGGER_SUCCESS);
	// msg
	ASSERT_TRUE(spLoggerPrintMsg(NULL) == SP_LOGGER_INVAlID_ARGUMENT);
	ASSERT_TRUE(spLoggerPrintMsg("") == SP_LOGGER_SUCCESS);

	spLoggerDestroy();
	return true;
}

int main() {
	RUN_TEST(basicLoggerTest);
	RUN_TEST(basicLoggerAllLevelsTest);
	RUN_TEST(nullArgLoggerTest);
	return 0;
}
