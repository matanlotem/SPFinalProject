#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "main_aux.h"

extern "C" {
#include "SPConsts.h"
}


int main(int argc, char* argv[]) {
	// init
	SP_CONFIG_MSG configMsg;
	SPConfig config = spInit(argc, argv);
	if (!config) return -1;

	// create imgageProc object - can throw exception
	try {
		sp::ImageProc imageProc(config);

		// pre-processing
		int* numOfFeatures;
		SPPoint*** featsDB = spPreprocessing(&numOfFeatures, imageProc, config);
		if (!featsDB) {
			spConfigDestroy(config);
			return -1;
		}

		// query and search similar images
		SPPoint** queryFeats;
		int queryNumOfFeatures = 0;
		char queryFilename[STR_LEN];
		int *similarImages;

		// allocate similar images array
		similarImages = (int*) malloc(sizeof(int)*spConfigGetNumOfSimilarImages(config, &configMsg));
		if (!similarImages || configMsg != SP_CONFIG_SUCCESS)
			spLoggerPrintError(ERRORMSG_ALLOCATION, __FILE__, __func__, __LINE__ - 2);

		// query and find similar images
		else {
			queryFeats = spQuery(&queryNumOfFeatures, queryFilename, imageProc);
			while (queryFeats) {
				// find
				if (spFindSimilarImages(similarImages, queryFeats, queryNumOfFeatures, featsDB, numOfFeatures, config) == -1)
					break;
				// show
				if (spShowResults(similarImages, queryFilename, imageProc, config) == -1)
					break;
				// new query
				queryFeats = spQuery(&queryNumOfFeatures, queryFilename, imageProc);
			}
		}

		// cleanup
		if (similarImages) free(similarImages);
		destroySPPoint1D(queryFeats, queryNumOfFeatures);
		destroySPPoint2D(featsDB,spConfigGetNumOfImages(config, &configMsg), numOfFeatures);
		spConfigDestroy(config);

	}
	catch (...) { // failed creating imageProc object
		spConfigDestroy(config);
		return -1;
	}

	return 0;

}
