extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main_aux.h"
}

#include "SPImageProc.h"

#define CONFIG_DEFAULT_FILE "spcbir.config"

#define ERRORMSG_CONFIG "Config file not defined"
#define ERRORMSG_UNKOWN "Unkown error!"
#define ERRORMSG_ALLOCATION "Allocation error"

#define ERRORMSG_INIT_USAGE "Invalid command line : use -c <config_filename>"
#define ERRORMSG_CONFIG_FILE "The configuration file %s could not be opened"
#define ERRORMSG_CONFIG_DEFAULT "The default configuration file %s could not be opened"
#define ERRORMSG_INIT_LOGGER "Failed initializing logger"

#define ERRORMSG_FEATS_LOAD_NOF "Invalid number of features"
#define ERRORMSG_FEATS_LOAD_OPEN "Could not open %s features file for reading"
#define ERRORMSG_FEATS_LOAD_FRMT "File format does not match number of features and PCA dimension"
#define INFOMSG_FEATS_LOAD_SUCCESS "Successfully loaded image %d features file"

#define ERRORMSG_FEATS_SAVE_NULL "Cannot save NULL features"
#define ERRORMSG_FEATS_INDEX "Index %d out of range"
#define ERRORMSG_FEATS_SAVE_OPEN "Could not open %s features file for writing"
#define INFOMSG_FEATS_SAVE_SUCCESS "Successfully saved image %d features file"

#define ERRORMSG_FEATS_GET "Failed extracting/loading image %d features"
#define INFOMSG_START_PRE "Starting preprocessing"
#define INFOMSG_DONE_PRE "Done preprocessing"


SPConfig spInit(int argc, char* argv[]) {
	// invalid arguments
	if ((argc != 1) && !(argc == 3 && strcmp(argv[1],"-c") == 0)) {
		printf("%s\n",ERRORMSG_INIT_USAGE);
		return NULL;
	}

	/*** parse arguments and load configuration file ***/
	char configFile[1024];
	char msg[1024];
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
	/**
	 * Frees memory of a 1D SPPoint array of size dim
	 * Assumes dim is the correct dimension of the array
	 */
	if (DB != NULL) {
		for (int i=0; i<dim; i++)
			spPointDestroy(DB[i]);
		free(DB);
	}
}

void destroySPPoint2D(SPPoint ***DB, int dim, int *dim2) {
	/**
	 * Frees memory of a 2D SPPoint array (a 1D array of size dim of 1D SPPoint arrays)
	 * The dimension of array DB[i] is dim2[i]
	 * Assumes given array dimensions are correct
	 * Frees dim2 as well
	 */
	assert(DB != NULL && dim2 == NULL);
	if (DB != NULL) {
		for (int i=0; i<dim; i++)
			destroySPPoint1D(DB[i],dim2[i]);
		free(DB);
	}
	if (dim2 != NULL) free(dim2);
}


SPPoint** spLoadFeaturesFile(int index, int* numOfFeatures, const SPConfig config) {
	// validate parameters
	if (numOfFeatures == NULL) {
		spLoggerPrintWarning(ERRORMSG_FEATS_LOAD_NOF,__FILE__,__func__,__LINE__);
		return NULL;
	}
	if (config == NULL) {
		spLoggerPrintWarning(ERRORMSG_CONFIG,__FILE__,__func__,__LINE__);
		return NULL;
	}

	// get features file filename
	SP_CONFIG_MSG configMsg;
	char msg[1024];
	char filename[1024];
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
	if (featsFile == NULL) {
		sprintf(msg,ERRORMSG_FEATS_LOAD_OPEN,filename);
		spLoggerPrintWarning(msg,__FILE__,__func__,__LINE__);
		return NULL;
	}

	// allocate featutres
	fscanf(featsFile,"%d\n", numOfFeatures);
	SPPoint** feats = (SPPoint**) malloc(sizeof(*feats) * *numOfFeatures);
	if (!feats) {
		spLoggerPrintError(ERRORMSG_ALLOCATION, __FILE__, __func__, __LINE__ - 2);
		fclose(featsFile);
		return NULL;
	}

	// allocate feature temporary array
	int PCADim = spConfigGetPCADim(config, &configMsg);
	double* arr = (double*) malloc(sizeof(double) * PCADim);
	if (!arr) {
		spLoggerPrintError(ERRORMSG_ALLOCATION, __FILE__, __func__, __LINE__ - 2);
		free(feats);
		fclose(featsFile);
		return NULL;
	}

	// read features data from file
	int i=0, j=0;
	while (fscanf(featsFile,"%lf ", arr + (j++))) {
		if (j == PCADim) {
			if (i++ >= *numOfFeatures) break; // overflow
			feats[i-1] = spPointCreate(arr,PCADim,index);
			j = 0;
		}
	}
	fclose(featsFile);
	free(arr);

	// to many or not enough data values
	if (i != *numOfFeatures || j != 0) {
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
	if (feats == NULL) {
		spLoggerPrintWarning(ERRORMSG_FEATS_SAVE_NULL,__FILE__,__func__,__LINE__);
		return;
	}
	if (config == NULL) {
		spLoggerPrintWarning(ERRORMSG_CONFIG,__FILE__,__func__,__LINE__);
		return;
	}

	// get features file filename
	SP_CONFIG_MSG configMsg;
	char msg[1024];
	char filename[1024];
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
	int PCADim = spConfigGetPCADim(config, &configMsg);
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

SPPoint*** spPreprocessing(int** NOFptr, const SPConfig config) {
	// validate config file
	if (config == NULL) {
		spLoggerPrintError(ERRORMSG_CONFIG, __FILE__, __func__, __LINE__);
		return NULL;
	}

	spLoggerPrintInfo(INFOMSG_START_PRE);
	char msg[1024];

	// allocate features DB
	SP_CONFIG_MSG configMsg;
	int numOfImages = spConfigGetNumOfImages(config, &configMsg);
	SPPoint*** featsDB = (SPPoint***) malloc(numOfImages*sizeof(SPPoint**));
	if (!featsDB) {
		spLoggerPrintError(ERRORMSG_ALLOCATION, __FILE__, __func__, __LINE__ - 2);
		return NULL;
	}

	// allocate numOfFeatures DB
	char imagePath[1024];
	int* numOfFeatures = (int*) malloc(numOfImages * sizeof(int));
	if (!numOfFeatures) {
		spLoggerPrintError(ERRORMSG_ALLOCATION, __FILE__, __func__, __LINE__ - 2);
		free(featsDB);
		return NULL;
	}

	// populate features DB
	sp::ImageProc imageProc(config);
	for (int i=0; i<numOfImages; i++) {
		// extract and save
		if (spConfigIsExtractionMode(config, &configMsg)) {
			spConfigGetImagePath(imagePath, config, i);
			numOfFeatures[i] = spConfigGetNumOfFeatures(config, &configMsg);
			try { // c++ code here because imageProc.getImageFeatures throws exceptions
				featsDB[i] = imageProc.getImageFeatures(imagePath, i, numOfFeatures + i);
			} catch (...) {
				// TODO: print error
				featsDB[i] = NULL;
			}
			spSaveFeaturesFile(i, featsDB[i], numOfFeatures[i], config);
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
	*NOFptr = numOfFeatures;

	spLoggerPrintInfo(INFOMSG_DONE_PRE);
	return featsDB;
}
