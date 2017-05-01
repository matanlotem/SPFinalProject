#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdbool>
#include "../main_aux.h"

extern "C" {
#include "unit_test_util.h" //SUPPORTING MACROS ASSERT_TRUE/ASSERT_FALSE etc..
#include "../SPConsts.h"
}

#define TEST_DIR "unit_tests/sp_complete/"

SPConfig spInitConfigFname(const char* configFilename) {
	char* initArgs[3];
	char inputFlag[3];
	initArgs[1] = inputFlag;
	initArgs[2] = (char*) configFilename;
	strcpy(inputFlag,"-c");
	return spInit(3, initArgs);
}


bool selfImageTestConfigFname(const char* configFname) {
	if (FULL_OUTPUT)
		printf("%s\n", configFname);

	// initialize everything
	SP_CONFIG_MSG configMsg;
	SPConfig config = spInitConfigFname(configFname);
	ASSERT_TRUE(config);
	sp::ImageProc imageProc(config);
	SPKDTreeNode* featsTree = spPreprocessing(imageProc, config);
	ASSERT_TRUE(featsTree);

	// get interesting parameters
	int numOfImages = spConfigGetNumOfImages(config, &configMsg);
	ASSERT_TRUE(configMsg == SP_CONFIG_SUCCESS);
	int numOfSimilarImages = spConfigGetNumOfSimilarImages(config, &configMsg);
	ASSERT_TRUE(configMsg == SP_CONFIG_SUCCESS);
	int* similarImages = (int*) malloc(sizeof(int) * numOfSimilarImages);
	ASSERT_TRUE(similarImages);
	int queryNumOfFeatures;
	char queryImageFilename[STR_LEN];
	SPPoint** queryFeats;


	// check each image has itself as closest image
	for (int i=0; i<numOfImages; i++) {
		ASSERT_TRUE(spConfigGetImagePath(queryImageFilename, config, i) == SP_CONFIG_SUCCESS);
		queryFeats = imageProc.getImageFeatures(queryImageFilename, 0, &queryNumOfFeatures);
		ASSERT_TRUE(queryFeats);
		ASSERT_TRUE(spFindSimilarImages(similarImages, queryFeats, queryNumOfFeatures, featsTree, config) == 0);
		ASSERT_TRUE(similarImages[0] == i);
		destroySPPoint1D(queryFeats, queryNumOfFeatures);

		// show results
		if (FULL_OUTPUT) {
			printf("%d:\t",i);
			for (int j=0; j<numOfSimilarImages; j++)
				printf("%d ", similarImages[j]);
			printf("\n");
		}
	}

	// cleanup
	spKDTreeDestroy(featsTree);
	free(similarImages);
	spConfigDestroy(config);
	spLoggerDestroy();

	return true;
}

bool selfImageTest() {
	ASSERT_TRUE(selfImageTestConfigFname(TEST_DIR "test_kNN-3_KDMethod-RANDOM.config"));
	ASSERT_TRUE(selfImageTestConfigFname(TEST_DIR "test_kNN-3_KDMethod-MAX_SPREAD.config"));
	ASSERT_TRUE(selfImageTestConfigFname(TEST_DIR "test_kNN-3_KDMethod-INCREMENTAL.config"));
	ASSERT_TRUE(selfImageTestConfigFname(TEST_DIR "test_kNN-25_KDMethod-RANDOM.config"));
	ASSERT_TRUE(selfImageTestConfigFname(TEST_DIR "test_kNN-25_KDMethod-MAX_SPREAD.config"));
	ASSERT_TRUE(selfImageTestConfigFname(TEST_DIR "test_kNN-25_KDMethod-INCREMENTAL.config"));
	return true;
}

int main(int argc, char* argv[]) {
	if (argc > 1) printf("%s\n",argv[0]);
	RUN_TEST(selfImageTest);
	return 0;
}
