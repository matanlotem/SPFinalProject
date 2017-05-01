#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "unit_test_util.h" //SUPPORTING MACROS ASSERT_TRUE/ASSERT_FALSE etc..
#include "../SPConfig.h"
#include "../SPConsts.h"

#define CONFIG_TEST_DIR "unit_tests/sp_config/"

//Config is not defined
static bool basicConfigTest() {
	SP_CONFIG_MSG msg;
	ASSERT_TRUE(spConfigGetNumOfFeatures(NULL, &msg) == -1);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);
	return true;
}

// Try loading a config file and check if got expected message
static bool createConfigMsg(const char* configFilename, SP_CONFIG_MSG expMsg) {
	SP_CONFIG_MSG msg;
	SPConfig config = spConfigCreate(configFilename,&msg);
	bool res;
	if (expMsg == SP_CONFIG_SUCCESS)
		res = (config != NULL && msg == expMsg);
	else
		res = (config == NULL && msg == expMsg);
	spConfigDestroy(config);
    return res;
}

static bool basicConfigCreationTest() {
	SP_CONFIG_MSG msg;

	// no config file
	ASSERT_TRUE(spConfigGetNumOfFeatures(NULL, &msg) == -1);
	ASSERT_TRUE(msg == SP_CONFIG_INVALID_ARGUMENT);

	// config filename not passed
	ASSERT_TRUE(createConfigMsg(NULL, SP_CONFIG_INVALID_ARGUMENT));

	// config filename doesn't exist
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "asd.config", SP_CONFIG_CANNOT_OPEN_FILE));

	// config filename exists and opened
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "basicConfigTest1.config", SP_CONFIG_SUCCESS));

	return true;
}

// Check what happens if mandatory arguments are missing
static bool missingArgConfigTest() {
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "missingArgDir.config", SP_CONFIG_MISSING_DIR));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "missingArgPrefix.config", SP_CONFIG_MISSING_PREFIX));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "missingArgSuffix.config", SP_CONFIG_MISSING_SUFFIX));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "missingArgNumImages.config", SP_CONFIG_MISSING_NUM_IMAGES));

	return true;
}

// Check what happens if invalid argument value is passed
static bool vaildArgConfigTest() {
	// integer arguments
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgNumOfFeatures.config", SP_CONFIG_INVALID_INTEGER));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgNumOfImages.config", SP_CONFIG_INVALID_INTEGER));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgNumOfSimilarImages.config", SP_CONFIG_INVALID_INTEGER));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgPCADimension1.config", SP_CONFIG_INVALID_INTEGER));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgPCADimension2.config", SP_CONFIG_INVALID_INTEGER));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgKNN.config", SP_CONFIG_INVALID_INTEGER));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgLoggerLevel1.config", SP_CONFIG_INVALID_INTEGER));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgLoggerLevel2.config", SP_CONFIG_INVALID_INTEGER));

	// boolean arguments
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgExtractionMode.config", SP_CONFIG_INVALID_BOOL));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgMinimalGUI.config", SP_CONFIG_INVALID_BOOL));

	// string arguments
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgImagesSuffix1.config", SP_CONFIG_INVALID_STRING));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgImagesSuffix2.config", SP_CONFIG_INVALID_STRING));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgImagesDirectory.config", SP_CONFIG_INVALID_STRING));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgImagesPrefix.config", SP_CONFIG_INVALID_STRING));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgPCAFilename.config", SP_CONFIG_INVALID_STRING));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgLoggerFilename.config", SP_CONFIG_INVALID_STRING));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "invalidArgKDTreeSplitMethod.config", SP_CONFIG_INVALID_STRING));

	//empty arguments
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "emptyArgNumOfFeatures.config", SP_CONFIG_INVALID_INTEGER));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "emptyArgExtractionMode.config", SP_CONFIG_INVALID_BOOL));
	ASSERT_TRUE(createConfigMsg(CONFIG_TEST_DIR "emptyArgImagesPrefix.config", SP_CONFIG_SUCCESS));

	return true;
}

// Validate default values
static bool defaultValConfigTest() {
	SP_CONFIG_MSG msg;
	SPConfig config = spConfigCreate(CONFIG_TEST_DIR "defaultConfigTest.config",&msg);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigIsExtractionMode(config, &msg) == SP_CONFIG_DEFAULT_EXTRACTION_MODE);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigMinimalGui(config, &msg) == SP_CONFIG_DEFAULT_MINIMAL_GUI);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigGetNumOfFeatures(config, &msg) == SP_CONFIG_DEFAULT_NUM_OF_FEATURES);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigGetNumOfSimilarImages(config, &msg) == SP_CONFIG_DEFAULT_NUM_OF_SIMILAR_IMAGES);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigGetPCADim(config, &msg) == SP_CONFIG_DEFAULT_PCA_DIMENSIONS);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	//SP_CONFIG_DEFAULT_KNN 1
	//SP_CONFIG_DEFAULT_KD_TREE_SPLIT_METHOD MAX_SPREAD
	//SP_CONFIG_DEFAULT_LOGGER_LEVEL 3
	//SP_CONFIG_DEFAULT_LOGGER_FILENAME "stdout"

	spConfigDestroy(config);
	return true;
}

static bool valueConfigTest() {
	SP_CONFIG_MSG msg;
	SPConfig config = spConfigCreate(CONFIG_TEST_DIR "testValueConfig.config",&msg);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	ASSERT_TRUE(spConfigIsExtractionMode(config, &msg) == false);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigMinimalGui(config, &msg) == true);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigGetNumOfFeatures(config, &msg) == 5);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigGetPCADim(config, &msg) == 11);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigGetNumOfImages(config, &msg) == 13);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);
	ASSERT_TRUE(spConfigGetNumOfSimilarImages(config, &msg) == 9);
	ASSERT_TRUE(msg == SP_CONFIG_SUCCESS);

	char output[STR_LEN];
	//TODO
	ASSERT_TRUE(spConfigGetImagePath(output, config, 3) == SP_CONFIG_SUCCESS);
	//ASSERT_TRUE(streq(output,""));
	ASSERT_TRUE(spConfigGetFeaturesPath(output, config, 3) == SP_CONFIG_SUCCESS);
	//ASSERT_TRUE(streq(output,""));
	ASSERT_TRUE(spConfigGetPCAPath(output, config) == SP_CONFIG_SUCCESS);
	//ASSERT_TRUE(streq(output,""));

	spConfigDestroy(config);
	return true;
}

static bool invalidConfigLineTest() {
	// no =
	// invalid arg
	//TODO
	return true;
}

static bool nullArgConfigTest() {
	//TODO
	return true;
}

static bool createLoggerConfigTest() {
	//TODO
	return true;
}

int main() {
	RUN_TEST(basicConfigTest);
	RUN_TEST(basicConfigCreationTest);
	RUN_TEST(missingArgConfigTest);
	RUN_TEST(vaildArgConfigTest);
	RUN_TEST(defaultValConfigTest);
	RUN_TEST(valueConfigTest);
	RUN_TEST(invalidConfigLineTest);
	RUN_TEST(nullArgConfigTest);
	RUN_TEST(createLoggerConfigTest);
	return 0;
}
