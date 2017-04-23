/*
 * SPConsts.h
 *
 *  Created on: Apr 20, 2017
 *      Author: Matan
 */

#ifndef SPCONSTS_H_
#define SPCONSTS_H_

#define STR_LEN 1025
#define CONFIG_DEFAULT_FILE "spcbir.config"
#define QUERY_EXIT_STR "<>"

// Configuration default values and constranits
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
#define SP_CONFIG_CONSTRAINT_IMAGES_PREFIX_VAL {".jpg",".png",".bmp",".gif"}
#define SP_FEATURES_SUFFIX ".feats"


// Error / Info messages
#define SP_CONFIG_INVAlID_LINE_MSG "Invalid configuration line"
#define SP_CONFIG_INVAlID_VAL_MSG "Invalid value - constraint not met"

#define ERRORMSG_NULL_ARGS "NULL arguments passed"

#define ERRORMSG_CONFIG "Config file not defined"
#define ERRORMSG_UNKOWN "Unkown error!"
#define ERRORMSG_ALLOCATION "Allocation error"
#define ERRORMSG_CONFIG_GET "Couldn't config parameter"

#define ERRORMSG_INIT_USAGE "Invalid command line : use -c <config_filename>"
#define ERRORMSG_CONFIG_FILE "The configuration file %s could not be opened"
#define ERRORMSG_CONFIG_DEFAULT "The default configuration file %s could not be opened"
#define ERRORMSG_INIT_LOGGER "Failed initializing logger"

//#define ERRORMSG_FEATS_LOAD_NOF "Invalid number of features"
#define ERRORMSG_FEATS_LOAD_OPEN "Could not open %s features file for reading"
#define ERRORMSG_FEATS_LOAD_FRMT "File format does not match number of features and PCA dimension"
#define INFOMSG_FEATS_LOAD_SUCCESS "Successfully loaded image %d features file"

//#define ERRORMSG_FEATS_SAVE_NULL "Cannot save NULL features"
#define ERRORMSG_FEATS_INDEX "Index %d out of range"
#define ERRORMSG_FEATS_SAVE_OPEN "Could not open %s features file for writing"
#define INFOMSG_FEATS_SAVE_SUCCESS "Successfully saved image %d features file"

#define ERRORMSG_FEATS_GET "Failed extracting/loading image %d features"
#define INFOMSG_START_PRE "Starting preprocessing"
#define INFOMSG_DONE_PRE "Done preprocessing"

#define OUTPUTMSG_EXITING "Exiting...\n"
#define OUTPUTMSG_QUERY "Please enter an image path:\n"
#define OUTPUTMSG_NON_MINIMAL_GUI "Best candidates for - %s - are:\n"


#endif /* SPCONSTS_H_ */
