#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "main_aux.h"

/*
 * returns true if the strings match
 */
bool streq(const char* str1, const char* str2) {
	return (strcmp(str1, str2) == 0);
}


SPConfig spInit(int argc, char* argv[]) {
	// invalid arguments
	if ((argc != 1) && !(argc == 3 && strcmp(argv[1],"-c") == 0)) {
		printf("%s\n",ERRORMSG_INIT_USAGE);
		return NULL;
	}

	/*** parse arguments and load configuration file ***/
	char configFile[STR_LEN];
	char msg[STR_LEN];
	if (argc == 1) { // no config file passed
		strcpy(configFile,CONFIG_DEFAULT_FILE);
		sprintf(msg, ERRORMSG_CONFIG_DEFAULT, configFile);
	}
	else { // config file passed as argument
		strcpy(configFile, argv[2]);
		sprintf(msg, ERRORMSG_CONFIG_FILE, configFile);
	}

	SP_CONFIG_MSG configMsg;
	SPConfig config = spConfigCreate(configFile, &configMsg);

	// configuration file load failed
	if (configMsg != SP_CONFIG_SUCCESS) {
		printf("%s\n",msg);
		spConfigDestroy(config);
		return NULL;
	}

	/*** load logger ***/
	SP_LOGGER_MSG loggerMsg;
	if (spConfigInitLogger(config, &loggerMsg) != SP_CONFIG_SUCCESS ||
			loggerMsg != SP_LOGGER_SUCCESS) {
		spConfigDestroy(config);
		spLoggerDestroy();
		printf("%s\n",ERRORMSG_INIT_LOGGER);
		return NULL;
	}

	return config;
}

void destroySPPoint1D(SPPoint **DB, int dim) {
	if (DB) {
		for (int i=0; i<dim; i++)
			spPointDestroy(DB[i]);
		free(DB);
	}
}

void destroySPPoint2D(SPPoint ***DB, int dim, int *dim2) {
	assert(!DB || dim2);
	if (DB) {
		for (int i=0; i<dim; i++)
			destroySPPoint1D(DB[i],dim2[i]);
		free(DB);
	}
	if (dim2) free(dim2);
}


SPPoint** spLoadFeaturesFile(int index, int* numOfFeatures, const SPConfig config) {
	// validate parameters
	if (!numOfFeatures || !config) {
		spLoggerPrintWarning(ERRORMSG_NULL_ARGS,__FILE__,__func__,__LINE__);
		return NULL;
	}

	// get features file filename
	SP_CONFIG_MSG configMsg;
	char msg[STR_LEN];
	char filename[STR_LEN];
	configMsg = spConfigGetFeaturesPath(filename, config, index);
	if (configMsg != SP_CONFIG_SUCCESS) {
		if (configMsg == SP_CONFIG_INDEX_OUT_OF_RANGE)
			sprintf(msg, ERRORMSG_FEATS_INDEX, index);
		else
			strcpy(msg,ERRORMSG_UNKOWN);
		spLoggerPrintWarning(msg,__FILE__,__func__,__LINE__);
		return NULL;
	}

	// open features file
	FILE* featsFile = fopen(filename, "r");
	if (!featsFile) {
		sprintf(msg,ERRORMSG_FEATS_LOAD_OPEN,filename);
		spLoggerPrintWarning(msg,__FILE__,__func__,__LINE__);
		return NULL;
	}

	// allocate featutres
	fscanf(featsFile,"%d\n", numOfFeatures);
	sprintf(msg, DEBUGMSG_FEATS_EXPECTED_NOF, *numOfFeatures);
	spLoggerPrintDebug(msg,__FILE__,__func__,__LINE__);

	SPPoint** feats = (SPPoint**) malloc(sizeof(*feats) * *numOfFeatures);
	if (!feats) {
		spLoggerPrintError(ERRORMSG_ALLOCATION, __FILE__, __func__, __LINE__ - 2);
		fclose(featsFile);
		return NULL;
	}

	// allocate feature temporary array
	int PCADim = spConfigGetPCADim(config, &configMsg); // ###no msg validation
	double* arr = (double*) malloc(sizeof(double) * PCADim);
	if (!arr) {
		spLoggerPrintError(ERRORMSG_ALLOCATION, __FILE__, __func__, __LINE__ - 2);
		free(feats);
		fclose(featsFile);
		return NULL;
	}

	// read features data from file
	int i=0, j=0;
	while (fscanf(featsFile,"%lf ", arr + (j++))>0) {
		if (j == PCADim) {
			if (i++ >= *numOfFeatures) break; // overflow
			feats[i-1] = spPointCreate(arr,PCADim,index);
			j = 0;
		}
	}
	fclose(featsFile);
	free(arr);

	// to many or not enough data values
	sprintf(msg, DEBUGMSG_FEATS_LOADED_NOF, i);
	spLoggerPrintDebug(msg,__FILE__,__func__,__LINE__);

	if (i != *numOfFeatures || j != 1) {
		spLoggerPrintError(ERRORMSG_FEATS_LOAD_FRMT,__FILE__,__func__,__LINE__);
		destroySPPoint1D(feats, *numOfFeatures);
		return NULL;
	}

	// success message
	sprintf(msg, INFOMSG_FEATS_LOAD_SUCCESS, index);
	spLoggerPrintInfo(msg);

	return feats;
}


/*
 * assume numOfFeatures is OK
 * assume PCADim is OK
 * no errors, just warnings
 */
void spSaveFeaturesFile(int index, SPPoint** feats, int numOfFeatures, const SPConfig config) {
	// validate parameters
	if (!feats || !config) {
		spLoggerPrintWarning(ERRORMSG_NULL_ARGS,__FILE__,__func__,__LINE__);
		return;
	}

	// get features file filename
	SP_CONFIG_MSG configMsg;
	char msg[STR_LEN];
	char filename[STR_LEN];
	configMsg = spConfigGetFeaturesPath(filename, config, index);
	if (configMsg != SP_CONFIG_SUCCESS) {
		if (configMsg == SP_CONFIG_INDEX_OUT_OF_RANGE)
			sprintf(msg, ERRORMSG_FEATS_INDEX, index);
		else
			strcpy(msg,ERRORMSG_UNKOWN);
		spLoggerPrintWarning(msg,__FILE__,__func__,__LINE__);
		return;
	}

	// open features file
	FILE* featsFile = fopen(filename, "w");
	if (!featsFile) {
		sprintf(msg,ERRORMSG_FEATS_SAVE_OPEN,filename);
		spLoggerPrintWarning(msg,__FILE__,__func__,__LINE__);
		return;
	}

	// write features data to file
	int PCADim = spConfigGetPCADim(config, &configMsg); // ###no msg validation
	fprintf(featsFile,"%d\n", numOfFeatures);
	for (int i=0; i<numOfFeatures; i++) {
		for (int j=0; j<PCADim; j++)
			fprintf(featsFile,"%lf ", spPointGetAxisCoor(feats[i],j));
		fprintf(featsFile,"\n");
	}

	fclose(featsFile);

	// success message
	sprintf(msg, INFOMSG_FEATS_SAVE_SUCCESS, index);
	spLoggerPrintInfo(msg);
}

SPKDTreeNode* spPreprocessing(sp::ImageProc imageProc, const SPConfig config) {
	// validate parameters
	if (!config) {
		spLoggerPrintError(ERRORMSG_NULL_ARGS, __FILE__, __func__, __LINE__);
		return NULL;
	}

	spLoggerPrintInfo(INFOMSG_START_PRE);
	char msg[STR_LEN];
	SP_CONFIG_MSG configMsg;

	// get number of images
	int numOfImages = spConfigGetNumOfImages(config, &configMsg);
	if (configMsg != SP_CONFIG_SUCCESS) {
		spLoggerPrintError(ERRORMSG_CONFIG_GET, __FILE__, __func__, __LINE__);
		return NULL;
	}
	// get kd tree split method
	KD_METHOD splitMethod = spConfigGetKDSplitMethod(config, &configMsg);
	if (configMsg != SP_CONFIG_SUCCESS) {
		spLoggerPrintError(ERRORMSG_CONFIG_GET, __FILE__, __func__, __LINE__);
		return NULL;
	}

	// allocate features DB
	SPPoint*** featsDB = (SPPoint***) malloc(numOfImages*sizeof(SPPoint**));
	if (!featsDB) {
		spLoggerPrintError(ERRORMSG_ALLOCATION, __FILE__, __func__, __LINE__ - 2);
		return NULL;
	}

	// allocate numOfFeatures DB
	char imagePath[STR_LEN];
	int* numOfFeatures = (int*) malloc(numOfImages * sizeof(int));
	if (!numOfFeatures) {
		spLoggerPrintError(ERRORMSG_ALLOCATION, __FILE__, __func__, __LINE__ - 2);
		free(featsDB);
		return NULL;
	}

	// populate features DB
	for (int i=0; i<numOfImages; i++) {
		// extract and save
		if (spConfigIsExtractionMode(config, &configMsg)) { // ###no msg validation
			if (spConfigGetImagePath(imagePath, config, i) != SP_CONFIG_SUCCESS) {
				// failed getting image path
				spLoggerPrintError(ERRORMSG_CONFIG_GET, __FILE__, __func__, __LINE__);
				featsDB[i] = NULL;
			}
			else {
				numOfFeatures[i] = spConfigGetNumOfFeatures(config, &configMsg);
				featsDB[i] = imageProc.getImageFeatures(imagePath, i, numOfFeatures + i);
				spSaveFeaturesFile(i, featsDB[i], numOfFeatures[i], config);
			}
		}

		// load
		else
			featsDB[i] = spLoadFeaturesFile(i, numOfFeatures + i, config);

		// failed extracting / loading
		if (!featsDB[i]) {
			sprintf(msg,ERRORMSG_FEATS_GET,i);
			spLoggerPrintError(msg, __FILE__, __func__, __LINE__);
			destroySPPoint2D(featsDB, i+1, numOfFeatures);
			return NULL;
		}
	}

	// create KD tree out of all features
	SPKDTreeNode* featsTree = fullKDTreeCreator(featsDB ,numOfImages, numOfFeatures, splitMethod);
	if (!featsTree) {
		spLoggerPrintError(ERRORMSG_KDTREE_CREATE, __FILE__, __func__, __LINE__);
		destroySPPoint2D(featsDB, numOfImages, numOfFeatures);
		return NULL;
	}

	// free featsDB but leave points (to be freed when tree is destroyed)
	for (int i=0; i<numOfImages; i++)
		free(featsDB[i]);
	free(featsDB);
	free(numOfFeatures);

	spLoggerPrintInfo(INFOMSG_DONE_PRE);
	return featsTree;
}

SPPoint** spQuery(int* queryNumOfFeatures, char* queryFilename, sp::ImageProc imageProc) {
	// validate parameters
	if (!queryNumOfFeatures || !queryFilename) {
		spLoggerPrintError(ERRORMSG_NULL_ARGS, __FILE__, __func__, __LINE__);
		return NULL;
	}

	// get query path from user
	printf(OUTPUTMSG_QUERY);
	fgets(queryFilename, STR_LEN, stdin);
	if (queryFilename[strlen(queryFilename)-1] == '\n') queryFilename[strlen(queryFilename)-1] = '\0';

	// if <> return NULL and print exit message
	if (streq(queryFilename,QUERY_EXIT_STR)) {
		printf(OUTPUTMSG_EXITING);
		return NULL;
	}

	// else get query features
	SPPoint** queryFeats = imageProc.getImageFeatures(queryFilename, 0, queryNumOfFeatures);
	if (!queryFeats) {
		printf(OUTPUTMSG_QUERY_PROCESS);
		printf(OUTPUTMSG_EXITING);
	}
	return queryFeats;
}

int spFindSimilarImages(int* similarImages, SPPoint** queryFeats, int queryNumOfFeatures,
		SPKDTreeNode* featsTree, const SPConfig config) {
	// validate parameters
	if (!similarImages || !queryFeats || !featsTree || !config) {
		spLoggerPrintError(ERRORMSG_NULL_ARGS, __FILE__, __func__, __LINE__);
		return -1;
	}

	// get configuration parameters
	SP_CONFIG_MSG msg;
	int numOfSimilarImages = spConfigGetNumOfSimilarImages(config, &msg);
	if (msg != SP_CONFIG_SUCCESS) {
		spLoggerPrintError(ERRORMSG_CONFIG_GET, __FILE__, __func__, __LINE__);
		return -1;
	}
	int kNN = spConfigGetKNN(config, &msg);
	if (msg != SP_CONFIG_SUCCESS) {
		spLoggerPrintError(ERRORMSG_CONFIG_GET, __FILE__, __func__, __LINE__);
		return -1;
	}
	int numOfImages = spConfigGetNumOfImages(config, &msg);
	if (msg != SP_CONFIG_SUCCESS) {
		spLoggerPrintError(ERRORMSG_CONFIG_GET, __FILE__, __func__, __LINE__);
		return -1;
	}

	// find nearest images
	if (closestImagesSearch(kNN, similarImages, numOfSimilarImages, queryFeats, queryNumOfFeatures, featsTree, numOfImages) == -1) {
		spLoggerPrintError(ERRORMSG_COLSEST_IMAGE_SEARCH, __FILE__, __func__, __LINE__);
		return -1;
	}

	return 0;
}

int spShowResults(int* similarImages, char* imageFilename, sp::ImageProc imageProc, const SPConfig config) {
	// validate parameters
	if (!similarImages || !imageFilename || !config) {
		spLoggerPrintError(ERRORMSG_NULL_ARGS, __FILE__, __func__, __LINE__);
		return -1;
	}

	SP_CONFIG_MSG msg;
	char imagePath[STR_LEN];

	// get minimal gui state
	bool minimalGui = spConfigMinimalGui(config, &msg);
	if (msg != SP_CONFIG_SUCCESS) {
		spLoggerPrintError(ERRORMSG_CONFIG_GET, __FILE__, __func__, __LINE__);
		return -1;
	}

	// get number of images
	int numOfSimilarImages = spConfigGetNumOfSimilarImages(config, &msg);
	if (msg != SP_CONFIG_SUCCESS) {
		spLoggerPrintError(ERRORMSG_CONFIG_GET, __FILE__, __func__, __LINE__);
		return -1;
	}

	// if not minimal gui - print general message
	if (!minimalGui)
		printf(OUTPUTMSG_NON_MINIMAL_GUI, imageFilename);

	// output for all images
	for (int i=0; i<numOfSimilarImages; i++) {
		// get image path
		if (spConfigGetImagePath(imagePath,config, similarImages[i]) != SP_CONFIG_SUCCESS) {
			spLoggerPrintError(ERRORMSG_CONFIG_GET, __FILE__, __func__, __LINE__);
			return -1;
		}
		// output for image
		if (minimalGui)
			imageProc.showImage(imagePath);
		else
			printf("%s\n",imagePath);
	}

	return 0;
}
