#ifndef MAIN_AUX_H_
#define MAIN_AUX_H_

#include "SPImageProc.h"
extern "C" {
#include "SPLogger.h"
#include "SPConfig.h"
#include "SPPoint.h"
#include "SPConsts.h"
#include "SPKDTree.h"
}



SPConfig spInit(int argc, char* argv[]);

/* Pre-process data structure for nearest image search
 *
 * @param NOFptr - return parameter - pointer to array holding number of features per image
 * @param imageProc - an open imageProc object for processing images
 * @param config - configuration structure
 *
 * @return KD tree containing all features
 * 		   returns NULL on failure
 */
SPKDTreeNode* spPreprocessing(sp::ImageProc imageProc, const SPConfig config);

/* Queries user for image and processes image features.
 *
 * @param queryNumOfFeatures - return parameter - pointer to number of found features
 * 							   in query image.
 * @param queryFilename - return paramater - string containing query image filename
 * @param imageProc - an open imageProc object for processing images
 *
 * @return SPPoint array containing all query image features
 * 		   returns NULL on failure.
 */
SPPoint** spQuery(int* queryNumOfFeatures, char* queryFilename, sp::ImageProc imageProc);

/* Finds k (specified in the configuration file) most similar images to query image
 *
 * @param similarImages - return parameter - array of size k containing the indices of
 * 						  the k closest images to the query image.
 * @param queryFeats - array of features of query image
 * @param queryNumOfFeatures - size of query image features array
 * @param featsTree - KD tree containing all features
 * @param config - configuration structure
 *
 * @return 0 on success, -1 otherwise
 */
int spFindSimilarImages(int* similarImages, SPPoint** queryFeats, int queryNumOfFeatures,
		SPKDTreeNode* featsTree, const SPConfig config);

/* Displays similar images results - has 2 modes:
 * 1. Minimal-Gui - graphicaly displays similar images. Press any key to move to next image
 * 2. No Minimal-Gui - prints similar images file names
 *
 * @param similarImages - array containing indices of most similar images
 * @param imageFilename - string containig query image filename (in order to display it)
 * @param imageProc - an open imageProc object for processing images
 * @param config - configuration structure
 *
 * @return 0 on success, -1 otherwise
 */
int spShowResults(int* similarImages, char* imageFilename,
		sp::ImageProc imageProc, const SPConfig config);

/* Frees memory of a 1D SPPoint array of size dim
 * Assumes dim is the correct dimension of the array
 *
 * @param DB - a 1D SPPoint array
 * @param dim - size of DB
 */
void destroySPPoint1D(SPPoint **DB, int dim);

/* Frees memory of a 2D SPPoint array (a 1D array of size dim of 1D SPPoint arrays)
 * The dimension of array DB[i] is dim2[i]
 * Assumes given array dimensions are correct
 * Frees dim2 as well
 *
 * @param DB - a 2D SPPoint array
 * @param dim - number of 1D SSPoint arrays in DB
 * @param dim2 - dimension of each 1D SSPoint array in DB
 * 				 asserts is not NULL if DB is not NULL
 */
void destroySPPoint2D(SPPoint ***DB, int dim, int *dim2);


#endif /* MAIN_AUX_H_ */
