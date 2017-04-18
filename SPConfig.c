#include <malloc.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "SPConfig.h"
#include "SPLogger.h"

// default values and limits
#define SP_CONFIG_DEFAULT_PCA_DIMENSIONS 20
#define SP_CONFIG_CONSTRAINT_PCA_DIMENSIONS_MIN 10
#define SP_CONFIG_CONSTRAINT_PCA_DIMENSIONS_MAX 28
#define SP_CONFIG_DEFAULT_PCA_FILENAME "pca.yml"
#define SP_CONFIG_DEFAULT_NUM_OF_FEATURES 100
#define SP_CONFIG_DEFAULT_EXTRACTION_MODE true
#define SP_CONFIG_DEFAULT_MINIMAL_GUI false
#define SP_CONFIG_DEFAULT_NUM_OF_SIMILAR_IMAGES 1
#define SP_CONFIG_DEFAULT_KNN 1
#define SP_CONFIG_DEFAULT_KD_TREE_SPLIT_METHOD MAX_SPREAD
#define SP_CONFIG_DEFAULT_LOGGER_LEVEL 3
#define SP_CONFIG_CONSTRAINT_LOGGER_LEVEL_MIN 1
#define SP_CONFIG_CONSTRAINT_LOGGER_LEVEL_MAX 4
#define SP_CONFIG_DEFAULT_LOGGER_FILENAME "stdout"
#define SP_CONFIG_CONSTRAINT_IMAGES_PREFIX_NUM 4
const char* SP_CONFIG_CONSTRAINT_IMAGES_PREFIX_VAL[] = {".jpg",".png",".bmp",".gif"};
#define SP_FEATURES_SUFFIX ".feats"


#define SP_CONFIG_INVAlID_LINE_MSG "Invalid configuration line"
#define SP_CONFIG_INVAlID_VAL_MSG "Invalid value - constraint not met"

typedef enum kd_method {
	RANDOM,
	MAX_SPREAD,
	INCREMENTAL
} KD_METHOD;

struct sp_config_t{
	char spImagesDirectory[1024];	// no spaces
	char spImagesPrefix[1024];		// no spaces
	char spImagesSuffix[1024];		// in {'.jpg','.png','.bmp','.gif'}
	int spNumOfImages;				// >0
	int spPCADimension;				// in [10,28]		default 20
	char spPCAFilename[1024];		// no spaces		default pca.yml
	int spNumOfFeatures;			// >0				default 100
	bool spExtractionMode;			// 					default true
	int spNumOfSimilarImages;		// >0				default 1
	KD_METHOD spKDTreeSplitMethod;	//					default MAX_SPREAD
	int spKNN;						// >0				default 1
	bool spMinimalGUI;				// 					default false
	int spLoggerLevel;				// in {1,2,3,4}		default 3
	char spLoggerFilename[1024];	// no spaces		default stdout (NULL)
};

/*
 * remove white-spaces from both ends of input and copy to output
 */
void trim(char* input, char** output, int len) {
	int start = 0, end = len-1;
	while (start < len && isspace(input[start]) != 0) start++;
	while (end>= 0 && isspace(input[end]) !=0) end--;

	*output = input + start;
	input[end+1] = '\0';
}


/*
 * split around
 */
SP_CONFIG_MSG splitTrim(char* input, char** output1, char** output2, char delimeter) {
	// find delimeter
	int mid = 0, len = (int) strlen(input);
	while (mid < len && input[mid] != delimeter) mid ++;

	// get first string
	trim(input, output1, mid);

	// delimeter not found
	if (mid == len)
		return SP_CONFIG_INVALID_LINE;

	// get second string
	trim(input+mid+1, output2, len - mid - 1);
	return SP_CONFIG_SUCCESS;
}

/*
 * returns true if the strings match
 */
bool streq(const char* str1, const char* str2) {
	return (strcmp(str1, str2) == 0);
}

/*
 * Parse string value and assign to configuration field.
 *
 * @param val - a string containing the configuration value
 * @param valptr - pointer to the configuration field
 * @param strConstraint - array of allowed string values
 * 		if NULL all values allowed
 * @param constraintLen - length of strConstraint
 *
 * @return	SP_CONFIG_INVALID_STRING if contains white spaces or not allowed values
 *			SP_CONFIG_SUCCESS - in case of success
 */
SP_CONFIG_MSG spConfigParseString(const char* val, char* valptr, char** strConstraint, int constraintLen) {
	// check white spaces
	if (strchr(val, ' ') != NULL || strchr(val, '\t'))
		return SP_CONFIG_INVALID_STRING;

	strcpy(valptr, val);

	// check allowed value
	if (strConstraint != NULL) {
		for (int i=0; i<constraintLen; i++)
			if (streq(val,strConstraint[i]))
				return SP_CONFIG_SUCCESS;
		return SP_CONFIG_INVALID_STRING;
	}

	return SP_CONFIG_SUCCESS;
}

/*
 * Parse integer value and assign to configuration field.
 *
 * @param val - a string containing the configuration value
 * @param valptr - pointer to the configuration field
 * @param minVal - minimal allowed value
 * @param maxVal - minimal allowed value
 *
 * @return	SP_CONFIG_INVALID_INTEGER if val cannot be parsed as integer
 * 										 or is out of the allowed range
 *			SP_CONFIG_SUCCESS - in case of success
 */
SP_CONFIG_MSG spConfigParseInt(const char *val, int *valptr, int minVal, int maxVal) {
	// validate is number
	for (int i=0; i < (int) strlen(val); i++)
		if (!isdigit(val[i]) && !(i=0 && strlen(val) > 1 && val[i] == '-'))
			return SP_CONFIG_INVALID_INTEGER;

	// check allowed range
	*valptr = atoi(val);
	if (*valptr < minVal || *valptr > maxVal)
		return SP_CONFIG_INVALID_INTEGER;

	return SP_CONFIG_SUCCESS;
}

/*
 * Parse boolean value and assign to configuration field.
 *
 * @param val - a string containing the configuration value
 * @param valptr - pointer to the configuration field
 *
 * @return	SP_CONFIG_INVALID_BOOL if val is not 'true' or 'false'
 *			SP_CONFIG_SUCCESS - in case of success
 */
SP_CONFIG_MSG spConfigParseBool(const char *val, bool *valptr) {
	if (streq(val,"true")) 			*valptr = true;
	else if (streq(val,"false"))	*valptr = false;
	else							return SP_CONFIG_INVALID_BOOL;
	return SP_CONFIG_SUCCESS;
}

/*
 * Parse KD_METHOD value and assign to configuration field.
 *
 * @param val - a string containing the configuration value
 * @param valptr - pointer to the configuration field
 *
 * @return	SP_CONFIG_INVALID_STRING if val is not one of the enum values
 *			SP_CONFIG_SUCCESS - in case of success
 */
SP_CONFIG_MSG spConfigParseKDEnum(const char *val, KD_METHOD *valptr) {
	if (streq(val,"RANDOM"))			*valptr = RANDOM;
	else if (streq(val,"MAX_SPREAD"))	*valptr = MAX_SPREAD;
	else if (streq(val,"INCREMENTAL"))	*valptr = INCREMENTAL;
	else								return SP_CONFIG_INVALID_STRING;
	return SP_CONFIG_SUCCESS;
}

SPConfig spConfigCreate(const char* filename, SP_CONFIG_MSG* msg) {
	/*** PARSER SETUP ***/
	/********************/
	*msg = SP_CONFIG_SUCCESS;
	if (filename == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return NULL;
	}

	FILE* configFile = fopen(filename, "r");
	if (configFile == NULL) {
		*msg = SP_CONFIG_CANNOT_OPEN_FILE;
		return NULL;
	}

	// allocate config file
	SPConfig config = NULL;
	config = (SPConfig) malloc(sizeof(*config));
	if (config == NULL) {
		*msg = SP_CONFIG_ALLOC_FAIL;
		fclose(configFile);
		return NULL;
	}

	// set default values
	config->spPCADimension		=	SP_CONFIG_DEFAULT_PCA_DIMENSIONS;
	config->spNumOfFeatures		=	SP_CONFIG_DEFAULT_NUM_OF_FEATURES;
	config->spExtractionMode	=	SP_CONFIG_DEFAULT_EXTRACTION_MODE;
	config->spMinimalGUI		=	SP_CONFIG_DEFAULT_MINIMAL_GUI;
	config->spNumOfSimilarImages=	SP_CONFIG_DEFAULT_NUM_OF_SIMILAR_IMAGES;
	config->spKNN				=	SP_CONFIG_DEFAULT_KNN;
	config->spKDTreeSplitMethod	=	SP_CONFIG_DEFAULT_KD_TREE_SPLIT_METHOD;
	config->spLoggerLevel		=	SP_CONFIG_DEFAULT_LOGGER_LEVEL;
	strcpy(config->spPCAFilename, SP_CONFIG_DEFAULT_PCA_FILENAME);
	strcpy(config->spLoggerFilename, SP_CONFIG_DEFAULT_LOGGER_FILENAME);

	// must set values
	bool setImagesDirectory = false, setImagesPrefix = false,
		 setImagesSuffix = false, setNumOfImages = false;

	/*** START PARSING ***/
	/*********************/
	char line[1024];//, var[1024], val[1024];
	char *var, *val;
	int lineNum = 0;
	while (fgets(line, sizeof(line), configFile) && *msg == SP_CONFIG_SUCCESS) {
		lineNum ++;
		*msg = splitTrim(line, &var, &val, '=');

		// comment/empty => skip to next line
		if (strlen(var) == 0 || var[0] == '#') {
			*msg = SP_CONFIG_SUCCESS;
			continue;
		}

		// no delimeter => error
		if (*msg != SP_CONFIG_SUCCESS) break;

		// spImagesDirectory
		else if (streq(var, "spImagesDirectory")) {
			*msg = spConfigParseString(val, (char*) config->spImagesDirectory, NULL, 0);
			setImagesDirectory = true;
		}

		// spImagesPrefix
		else if (streq(var, "spImagesPrefix")) {
			*msg = spConfigParseString(val, config->spImagesPrefix, NULL, 0);
			setImagesPrefix = true;
		}

		// spImagesSuffix
		else if (streq(var, "spImagesSuffix")) {
			*msg = spConfigParseString(val, config->spImagesSuffix,
									   (char**) SP_CONFIG_CONSTRAINT_IMAGES_PREFIX_VAL,
									   SP_CONFIG_CONSTRAINT_IMAGES_PREFIX_NUM);
			setImagesSuffix = true;
		}

		// spNumOfImages
		else if (streq(var, "spNumOfImages")) {
			*msg = spConfigParseInt(val, &(config->spNumOfImages), 1, INT_MAX);
			setNumOfImages = true;
		}

		// spPCADimension
		else if (streq(var, "spPCADimension"))
			*msg = spConfigParseInt(val, &(config->spPCADimension),
									SP_CONFIG_CONSTRAINT_PCA_DIMENSIONS_MIN,
									SP_CONFIG_CONSTRAINT_PCA_DIMENSIONS_MAX);
		// spPCAFilename
		else if (streq(var, "spPCAFilename"))
			*msg = spConfigParseString(val, config->spPCAFilename, NULL, 0);

		// spNumOfFeatures
		else if (streq(var, "spNumOfFeatures"))
			*msg = spConfigParseInt(val, &(config->spNumOfFeatures),1, INT_MAX);

		// spExtractionMode
		else if (streq(var, "spExtractionMode"))
			*msg = spConfigParseBool(val, &(config->spExtractionMode));

		// spNumOfSimilarImages
		else if (streq(var, "spNumOfSimilarImages"))
			*msg = spConfigParseInt(val, &(config->spNumOfSimilarImages),1, INT_MAX);

		// spKDTreeSplitMethod
		else if (streq(var, "spKDTreeSplitMethod"))
			*msg = spConfigParseKDEnum(val, &(config->spKDTreeSplitMethod));

		// spKNN
		else if (streq(var, "spKNN"))
			*msg = spConfigParseInt(val, &(config->spKNN), 1, INT_MAX);

		// spMinimalGUI
		else if (streq(var, "spMinimalGUI"))
			*msg = spConfigParseBool(val, &(config->spMinimalGUI));

		// spLoggerLevel
		else if (streq(var, "spLoggerLevel"))
			*msg =spConfigParseInt(val, &(config->spLoggerLevel), 1, INT_MAX);

		// spLoggerFilename
		else if (streq(var, "spLoggerFilename"))
			*msg = spConfigParseString(val, config->spLoggerFilename,NULL, 0);

		// undefined message
		else
			*msg = SP_CONFIG_INVALID_LINE;
	}
	fclose(configFile);

	/*** HANDLE ERRORS ***/
	/*********************/
	// parsing error
	char *errmsg = NULL;
	if (*msg == SP_CONFIG_INVALID_LINE) errmsg = SP_CONFIG_INVAlID_LINE_MSG;
	else if (*msg == SP_CONFIG_INVALID_INTEGER ||
			 *msg == SP_CONFIG_INVALID_BOOL ||
			 *msg == SP_CONFIG_INVALID_STRING)
		errmsg = SP_CONFIG_INVAlID_VAL_MSG;

	// missing non default variable
	else if (!setImagesDirectory) *msg = SP_CONFIG_MISSING_DIR;
	else if (!setImagesPrefix) *msg = SP_CONFIG_MISSING_PREFIX;
	else if (!setImagesSuffix) *msg = SP_CONFIG_MISSING_SUFFIX;
	else if (!setNumOfImages) *msg = SP_CONFIG_MISSING_NUM_IMAGES;

	// print error message
	if (*msg != SP_CONFIG_SUCCESS) {
		printf("File: %s\n", filename);
		printf("Line: %d\n", lineNum);
		if (errmsg != NULL) printf("Message: %s\n", errmsg);
		spConfigDestroy(config);
		config = NULL;
	}

	return config;
}


/***********************************************************************/
bool spConfigValidate(const SPConfig config, SP_CONFIG_MSG* msg) {
	assert(msg != NULL);
	if (config != NULL) {
		*msg = SP_CONFIG_SUCCESS;
		return true;
	}
	else {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return false;
	}
}

bool spConfigIsExtractionMode(const SPConfig config, SP_CONFIG_MSG* msg) {
	return (spConfigValidate(config, msg) && config->spExtractionMode);
}

bool spConfigMinimalGui(const SPConfig config, SP_CONFIG_MSG* msg) {
	return (spConfigValidate(config, msg) && config->spMinimalGUI);
}

int spConfigGetNumOfImages(const SPConfig config, SP_CONFIG_MSG* msg) {
	if (spConfigValidate(config, msg))
		return config->spNumOfImages;
	return -1;
}

int spConfigGetNumOfFeatures(const SPConfig config, SP_CONFIG_MSG* msg) {
	if (spConfigValidate(config, msg))
		return config->spNumOfFeatures;
	return -1;
}

int spConfigGetPCADim(const SPConfig config, SP_CONFIG_MSG* msg) {
	if (spConfigValidate(config, msg))
		return config->spPCADimension;
	return -1;
}

SP_CONFIG_MSG spConfigGetPath (char* filePath, const SPConfig config, int index, char* suffix) {
	if (config == NULL || filePath == NULL)
		return SP_CONFIG_INVALID_ARGUMENT;

	if (index < 0 || index >= config->spNumOfImages)
		return SP_CONFIG_INDEX_OUT_OF_RANGE;

	sprintf(filePath,"%s%s%d%s",config->spImagesDirectory, config->spImagesPrefix,
			index, suffix);
	return SP_CONFIG_SUCCESS;
}

SP_CONFIG_MSG spConfigGetImagePath(char* imagePath, const SPConfig config, int index) {
	return spConfigGetPath(imagePath, config, index, config->spImagesSuffix);
}

SP_CONFIG_MSG spConfigGetFeaturesPath(char* featsPath, const SPConfig config, int index) {
	return spConfigGetPath(featsPath, config, index, SP_FEATURES_SUFFIX);
}


SP_CONFIG_MSG spConfigGetPCAPath(char* pcaPath, const SPConfig config) {
	if (config == NULL || pcaPath == NULL)
		return SP_CONFIG_INVALID_ARGUMENT;
	sprintf(pcaPath,"%s%s",config->spImagesDirectory, config->spPCAFilename);
	return SP_CONFIG_SUCCESS;
}

SP_CONFIG_MSG spConfigInitLogger(const SPConfig config, SP_LOGGER_MSG* loggerMsg) {
	if (config == NULL || loggerMsg == NULL)
		return SP_CONFIG_INVALID_ARGUMENT;

	// logger level
	SP_LOGGER_LEVEL loggerLevel;
	switch (config->spLoggerLevel) {
	case 1:
		loggerLevel = SP_LOGGER_ERROR_LEVEL;
		break;
	case 2:
		loggerLevel = SP_LOGGER_WARNING_ERROR_LEVEL;
		break;
	case 3:
		loggerLevel = SP_LOGGER_INFO_WARNING_ERROR_LEVEL;
		break;
	case 4:
		loggerLevel = SP_LOGGER_DEBUG_INFO_WARNING_ERROR_LEVEL;
		break;
	}

	// logger file (or stdout)
	char *loggerFile = NULL;
	if (!streq(config->spLoggerFilename,"stdout"))
		loggerFile = config->spLoggerFilename;

	// create logger
	*loggerMsg = spLoggerCreate(loggerFile, loggerLevel);
	return SP_CONFIG_SUCCESS;
}

void spConfigDestroy(SPConfig config) {
	if (config != NULL) free(config);
}
