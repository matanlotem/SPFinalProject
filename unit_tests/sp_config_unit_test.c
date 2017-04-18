#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "unit_test_util.h" //SUPPORTING MACROS ASSERT_TRUE/ASSERT_FALSE etc..
#include "../SPConfig.h"


//Config is not defined
static bool basicConfigTest() {
	SP_CONFIG_MSG msg;
	ASSERT_TRUE(spConfigGetNumOfFeatures(NULL, &msg) == -1);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);
	return true;
}

static bool basicConfigCreationTest() {
	SP_CONFIG_MSG msg;
	// no config file
	ASSERT_TRUE(spConfigGetNumOfFeatures(NULL, &msg) == -1);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	SPConfig config;

	// config filename not passed
	ASSERT_TRUE(spConfigCreate(NULL,&msg) == NULL);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	// config filename doesn't exist
	ASSERT_TRUE(spConfigCreate("unit_tests/sp_config/asd.config",&msg) == NULL);
	ASSERT_TRUE(msg == SP_CONFIG_CANNOT_OPEN_FILE);

	// config filename exists and opened
	config = spConfigCreate("unit_tests/sp_config/basicConfigTest1.config",&msg);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	// destroy
	spConfigDestroy(config);
	return true;
}

int main() {
	RUN_TEST(basicConfigTest);
	RUN_TEST(basicConfigCreationTest);
	return 0;
}
