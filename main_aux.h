#ifndef MAIN_AUX_H_
#define MAIN_AUX_H_

#include "SPImageProc.h"
extern "C" {
#include "SPLogger.h"
#include "SPConfig.h"
#include "SPPoint.h"
#include "SPConsts.h"
}



SPConfig spInit(int argc, char* argv[]);

SPPoint*** spPreprocessing(int** NOFptr, sp::ImageProc imageProc, const SPConfig config);

SPPoint** spQuery(int* queryNumberOfFiles, char* queryFilename, sp::ImageProc imageProc);

int spFindSimilarImages(int* similarImages, SPPoint** queryFeats, int queryNumOfFeatures,
		SPPoint*** featsDB,int* numOfFeatures, const SPConfig config);

int spShowResults(int* similarImages, char* imageFilename,
		sp::ImageProc imageProc, const SPConfig config);

void destroySPPoint1D(SPPoint **DB, int dim);

void destroySPPoint2D(SPPoint ***DB, int dim, int *dim2);


#endif /* MAIN_AUX_H_ */
