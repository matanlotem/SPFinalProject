#include <malloc.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "SPPoint.h"
#include "SPKDArray.h"
#include "SPKDTree.h"
#include "SPBPriorityQueue.h"
#include "SPConfig.h"
#include "SPLogger.h"
#include "SPConsts.h"

/**
 * SPKDTree Summary
 * A kd tree is a binary tree that organises points with k dimensions.
 * Each non-leaf node represents a group of points, and splits the group in half around the median coordinate
 * in a dimension chosen by one of 3 methods. The left child represents the half with the lower values in that dimension,
 * while the right child represents the group with the higher values.
 * The root node represents the full array of points. The leaves contain the points themselves.
 * The different splitting methods are:
 * INCREMENTAL, meaning the dimension increases by 1 for each level.
 * RANDOM, meaning the dimension is randomly selected.
 * MAX_SPREAD, meaning the dimension in which the points cover the most distance is chosen.
 *
 * The purpose of the kd tree in this project is to easily find points that are close, in terms of distance,
 * to a target point.
 * The points are ordered differently by each dimension, and each
 * possible order is saved in the KD Array as an array.
 *
 * The following functions are supported:
 *
 * spKDTreeInit            	    - Initializes a KD tree based on an array of points, and splitting method.
 * spKDTreeInitRecursion 		- The recursion function used in spKDTreeInit.
 * kNearestNeighboursTree		- Fills a bounded priority queue with the closest points to a target point.
 * kNearestNeighboursRecursion	- The recursion function used in kNearestNeighboursTree.
 * minDistanceSquared		    - Calculates the minimal distance from a target point to an area within defined limits.
 * spKDTreeDestroy     		    - Frees all allocated memory in a KD tree.
 * fullKDTreeCreator    		- Initializes a KD tree containing the features of all the images. Uses spKDTreeInit.
 * closestImagesSearch 	        - Finds the closest points to all features of a target image, and returns the indices
 *                                of the images with the highest number of similar features. Uses kNearestNeighboursTree.
 *
 * kNearestNeighbours     		- Unused in main project, used for checking
 *
 */

/** Type for defining the tree node **/
struct kd_tree_node_t {
	SPPoint* data; /* If the node is a leaf, data is the pointer to the point it represents */
	SPKDTreeNode* left; /* The left child node */
	SPKDTreeNode* right; /* The right child node */
	int dim; /* The dimension which the kd array was split by at this node */
	double val; /* The median value around which the kd array was split by at this node */
};

/**
 * Initializes a new KD tree based on inputed point array and size of array.
 * First, a kd array is created, then it is split recursively using splitMethod to determine next split dimension.
 * A new tree node is created with each split, saving the split dimension and the median value in that dimension,
 * which the array split around. The left child of the node is the node created with the left kd array after the split,
 * and the right child of the node is the node created with the right kd array after the split.
 * When the recursive function is called for a kd array of 1 point, it is a leaf and
 * saves the pointer to the point.
 *
 * @param splitMethod - the method used to determine the split dimension
 * @param pointsArray - the array of pointers to points
 * @param pointsArraySize - the number of pointers to points
 *
 * @return NULL in case of allocation failure occurred OR pointsArray is NULL
 * Otherwise, the root node of the new tree is returned
 */
SPKDTreeNode* spKDTreeInit(KD_METHOD splitMethod , SPPoint** pointsArray, int pointsArraySize){
	if(pointsArray == NULL || pointsArraySize < 1){
        spLoggerPrintError(ERRORMSG_NULL_ARGS,__FILE__,__func__,__LINE__);
		return NULL;
	}
	SPKDArray* kdA = spKDArrayInit(pointsArray, pointsArraySize);
	if(kdA == NULL)
		return NULL;
    return spKDTreeInitRecursion (splitMethod, kdA, 0);
}

/**
 * The recursion function used to create the kd tree.
 * The recursion method is explained in the description of spKDTreeInit.
 * The inputed kd array is assumed to exist, not NULL.
 *
 * @param splitMethod - the method used to determine the split dimension
 * @param kdA - the kd array to be split, or (if it is of size 1) the array containing the point the leaf represents.
 * @param coorSplit - the previous split dimension (used in the INCREMENTAL method).
 *
 * @return NULL in case of allocation failure occurred
 * Otherwise, the new node of the tree is returned
 */
SPKDTreeNode* spKDTreeInitRecursion(KD_METHOD splitMethod , SPKDArray* kdA, int coorSplit){
	SPKDTreeNode* newNode = (SPKDTreeNode*) malloc(sizeof(*newNode));
	if(newNode == NULL){
        spLoggerPrintError(ERRORMSG_ALLOCATION, __FILE__, __func__, __LINE__ );
		return NULL;
	}
	if(spKDArrayGetSize(kdA) == 1){ /* Leaf initialisation */
		newNode->left = NULL;
		newNode->right = NULL;
		newNode->dim = -1;
		newNode->val = 0;
		newNode->data = (spKDArrayGetArray(kdA))[0];
		spKDArrayDestroy(kdA);
	}
	else{
		int n = spKDArrayGetSize(kdA);
		int d = spKDArrayGetDimension(kdA);
		if(splitMethod == INCREMENTAL) /* INCREMENTAL method (adds 1 to previous coorSplit) */
		{
			coorSplit = coorSplit+1;
			if(coorSplit > d)
				coorSplit = 1;
		}
		if(splitMethod == RANDOM) /* RANDOM method (random coorSplit) */
		{
			coorSplit = (rand() % d) + 1;
		}
		if(splitMethod == MAX_SPREAD) /* MAX_SPREAD method (coorSplit will be the dimension with the largest range of points) */
		{
            double maxSpread = 0;
            double currentSpread = 0;
			coorSplit = 1;
			for(int i = 0; i<d; i++){
				currentSpread = spPointGetAxisCoor((spKDArrayGetArray(kdA))[(spKDArrayGetIndicesByDim(kdA, i+1))[n-1]],i) - spPointGetAxisCoor((spKDArrayGetArray(kdA))[(spKDArrayGetIndicesByDim(kdA, i+1))[0]],i);
				if(maxSpread < currentSpread){
					maxSpread = currentSpread;
					coorSplit = i+1;
				}
			}
		}
		SPKDArray** kdASplit = spKDArraySplit(kdA, coorSplit); /* Split the array by dimension coorSplit */
		newNode->dim = coorSplit;
		int medianIndex = n;
		if(n % 2 == 1)
			medianIndex = medianIndex-1;
		medianIndex = (int)(medianIndex/2); /* newNode->val is set as the coordinate of the middle point (index medianIndex) in dimension coorSplit */
		newNode->val = spPointGetAxisCoor((spKDArrayGetArray(kdA))[(spKDArrayGetIndicesByDim(kdA, coorSplit))[medianIndex]],coorSplit-1);
		newNode->data = NULL; /* NULL data marks node, non-leaf */
		newNode->left = spKDTreeInitRecursion(splitMethod, kdASplit[0], coorSplit); /* Left child recursion, with left kd array */
		newNode->right = spKDTreeInitRecursion(splitMethod, kdASplit[1], coorSplit); /* Right child recursion, with right kd array */
		free(kdASplit);
		spKDArrayDestroy(kdA);
	}
    return newNode;
}

/**
 * This function searches the inputed kd tree for the closest points to an inputed target point.
 * Each point found is a feature in an image with an index, and that index as well as the distance squared is entered
 * into the bounded minimum priority queue bpq, where the priority is the distance squared and the lower it is the better.
 * Arrays to hold the limits covered by the kd subtrees are defined here, and their addresses are
 * sent into the recursive function kNearestNeighboursTree as well as the address of the tree and the queue.
 *
 * @param bpq - the bounded priority queue to fill
 * @param root - the root node of the tree to search
 * @param targetPoint - the point, or feature, that is being searched for in the other images
 *
 * @return -2 in case of allocation failure occurred. -1 in case bpq, root or targetNode are NULL.
 * Otherwise, 1 is returned.
 */
int kNearestNeighboursTree(SPBPQueue* bpq , SPKDTreeNode* root, SPPoint* targetPoint){
	if(root == NULL || targetPoint == NULL || bpq == NULL){
		spLoggerPrintError(ERRORMSG_NULL_ARGS,__FILE__,__func__,__LINE__);
		return -1;
	}
    double* lowLimit = (double*) malloc(spPointGetDimension(targetPoint) * sizeof(double)); /* These arrays are used in the recursion to mark limits */
    double* highLimit = (double*) malloc(spPointGetDimension(targetPoint) * sizeof(double));
    int* lowLimitUse = (int*) malloc(spPointGetDimension(targetPoint) * sizeof(int));
    int* highLimitUse = (int*) malloc(spPointGetDimension(targetPoint) * sizeof(int));
	if(highLimit == NULL || lowLimit == NULL || highLimitUse == NULL || lowLimitUse == NULL){
        spLoggerPrintError(ERRORMSG_ALLOCATION, __FILE__, __func__, __LINE__ );
        if(bpq != NULL)
            spBPQueueDestroy(bpq);
        if(highLimit != NULL)
            free(highLimit);
        if(lowLimit != NULL)
            free(lowLimit);
        if(highLimitUse != NULL)
            free(highLimitUse);
        if(lowLimitUse != NULL)
            free(lowLimitUse);
        return -2;
	}
    for(int i = 0; i<spPointGetDimension(targetPoint) ; i++){
        highLimit[i] = 0; /* highLimit[i] is the highest possible value of coordinate i in the current kd subtree in kNearestNeighboursRecursion */
        lowLimit[i] = 0; /* lowLimit[i] is the lowest possible value of coordinate i in the current kd subtree in kNearestNeighboursRecursion */
        highLimitUse[i] = 0; /* highLimitUse[i] is 0 if there is no limit on the highest possible value of coordinate i in the current kd subtree in kNearestNeighboursRecursion */
        lowLimitUse[i] = 0; /* lowLimitUse[i] is 0 if there is no limit on the lowest possible value of coordinate i in the current kd subtree in kNearestNeighboursRecursion */
    }
    kNearestNeighboursRecursion(bpq, root, targetPoint, highLimit, lowLimit, highLimitUse, lowLimitUse); /* Recursion function */
    free(highLimit); /* Freeing the allocated memory */
    free(lowLimit);
    free(highLimitUse);
    free(lowLimitUse);
	return 1;
}

/**
 * This function recursively travels along the kd tree and marks the limits defined by each subtree.
 * If the current node is a leaf, the index of the image containing the point it holds is sent to be added
 * to the priority queue, with the priority being the squared distance from the target point.
 *
 * If the current node is split by dimension with index currentDimIndex, the current limits of that dimension
 * are saved.
 * Next, the limits are set to match the limits of the left child subtree:
 * highLimitUse[currentDimIndex] is set to 1, meaning there is a top value.
 * highLimit[currentDimIndex] is set to the median value saved in the node,
 * meaning all points in the left subtree have a coordinate that is lower than the limit in the currentDimIndex dimension.
 * Next, if the queue is full, the minimal squared distance from those limits to the target point is calculated,
 * and if that squared distance is bigger than the maximal squared distance in the queue, that subtree is skipped.
 * If it is not skipped, the functoin is called on the left chid.
 *
 * After this, highLimitUse[currentDimIndex] highLimit[currentDimIndex] are restored to their previous values.
 * The limits are set to match the limits of the left child subtree:
 * lowLimitUse[currentDimIndex] is set to 1, meaning there is a bottom value.
 * lowLimit[currentDimIndex] is set to the median value saved in the node,
 * meaning all points in the right subtree have a coordinate that is higher than the limit in the currentDimIndex dimension.
 * Next, if the queue is full, the minimal squared distance from those limits to the target point is calculated,
 * and if that squared distance is bigger than the maximal squared distance in the queue, that subtree is skipped.
 * If it is not skipped, the function is called on the right chid.
 *
 * @param bpq - the bounded priority queue to fill
 * @param curr - the current node of the tree, the root of the current subtree
 * @param targetPoint - the point, or feature, that is being searched for
 * @param highLimit - the array that contains the maximum value for each dimension of the points in the subtree
 * @param lowLimit - the array that contains the minimum value for each dimension of the points in the subtree
 * @param highLimitUse - the array that marks if there is a maximum value for each dimension of the points in the subtree
 * @param lowLimitUse - the array that marks if there is a minimum value for each dimension of the points in the subtree
 *
 */
void kNearestNeighboursRecursion(SPBPQueue* bpq, SPKDTreeNode* curr, SPPoint* targetPoint, double* highLimit, double* lowLimit, int* highLimitUse, int* lowLimitUse){
    if(curr != NULL){
        if(curr->data != NULL){ /* If root is a leaf, try to add the index of the point and its distance from targetPoint to the queue */
            spBPQueueEnqueue(bpq, spPointGetIndex(curr->data), spPointL2SquaredDistance(targetPoint, curr->data));
        }
        else{
            bool cont = true; /* If cont becomes false, the next subtree will be skipped */
            int currentDimIndex = curr->dim -1; /* The indexes are 0 to d-1, while dim are 1 to d */
            double currentLowLimit = lowLimit[currentDimIndex]; /* The current limits of the splitting dimension are saved */
            double currentHighLimit = highLimit[currentDimIndex];
            int currentLowLimitUse = lowLimitUse[currentDimIndex];
            int currentHighLimitUse = highLimitUse[currentDimIndex];

            highLimit[currentDimIndex] = curr->val; /* The limits are changed to those of the left subtree */
            highLimitUse[currentDimIndex] = 1;
            if(spBPQueueIsFull(bpq) == true){
                if(minDistanceSquared(targetPoint, highLimit, lowLimit, highLimitUse, lowLimitUse) >= spBPQueueMaxValue(bpq))
                    cont = false; /* The left subtree is skipped only if the distance between the target point and the closest  */
            } /* point within the limits, is bigger than the distance between the target point and the furthest point in the full queue. */
            if(cont == true){ /* Recursion on the left subtree */
                kNearestNeighboursRecursion(bpq, curr->left, targetPoint, highLimit, lowLimit, highLimitUse, lowLimitUse);
            }
            highLimit[currentDimIndex] = currentHighLimit; /* The limits of the splitting dimension are restored */
            highLimitUse[currentDimIndex] = currentHighLimitUse;
            cont = true;

            lowLimit[currentDimIndex] = curr->val; /* The limits are changed to those of the right subtree */
            lowLimitUse[currentDimIndex] = 1;
            if(spBPQueueIsFull(bpq) == true){
                if(minDistanceSquared(targetPoint, highLimit, lowLimit, highLimitUse, lowLimitUse) >= spBPQueueMaxValue(bpq))
                    cont = false; /* The right subtree is skipped only if the distance between the target point and the closest  */
            } /* point within the limits, is bigger than the distance between the target point and the furthest point in the full queue. */
            if(cont == true){ /* Recursion on the right subtree */
                kNearestNeighboursRecursion(bpq, curr->right, targetPoint, highLimit, lowLimit, highLimitUse, lowLimitUse);
            }
            lowLimit[currentDimIndex] = currentLowLimit; /* The limits of the splitting dimension are restored */
            lowLimitUse[currentDimIndex] = currentLowLimitUse;
        }
    }
}

/**
 * This function calculates the distance squared from the target point to the closest point in the limits.
 * For each dimension that is limited, if the target point is outside the limit, the difference between
 * the closest edge of the limit and the relevant coordinate of the target point is squared and added to the result.
 *
 * @param targetPoint - the point, or feature, that is being searched for
 * @param highLimit - the array that contains the maximum value for each dimension
 * @param lowLimit - the array that contains the minimum value for each dimension
 * @param highLimitUse - the array that marks if there is a maximum value for each dimension
 * @param lowLimitUse - the array that marks if there is a minimum value for each dimension
 *
 * @return the minimum squared distance from the target point to the closest point in the limits.
 */
double minDistanceSquared(SPPoint* targetPoint, double* highLimit, double* lowLimit, int* highLimitUse, int* lowLimitUse){
    if(targetPoint == NULL || highLimit == NULL || lowLimit == NULL || highLimitUse == NULL || lowLimitUse == NULL){
    	spLoggerPrintError(ERRORMSG_NULL_ARGS,__FILE__,__func__,__LINE__);
        return 0;
    }
    double res = 0;
    for(int i = 0; i<spPointGetDimension(targetPoint) ; i++){
        if(lowLimitUse[i] == 1){ /* res will be more than 0 in dimensions in which there are limits and targetPoint is outside the limits */
            if(spPointGetAxisCoor(targetPoint, i) < lowLimit[i]){
                res = res+ (lowLimit[i] - spPointGetAxisCoor(targetPoint, i))*(lowLimit[i] - spPointGetAxisCoor(targetPoint, i));
            }
        }
        if(highLimitUse[i] == 1){
            if(spPointGetAxisCoor(targetPoint, i) > highLimit[i]){
                res = res+ (spPointGetAxisCoor(targetPoint, i) - highLimit[i])*(spPointGetAxisCoor(targetPoint, i) - highLimit[i]);
            }
        }
    }
    return res;
}

/**
 * Frees all allocated memory of kd tree. It is used recursively on each child.
 *
 * @param curr - the tree node to free
 */
void spKDTreeDestroy(SPKDTreeNode* curr){
    if (curr != NULL && curr->data != NULL) {
        spKDTreeDestroy(curr->left);
        spKDTreeDestroy(curr->right);
        free(curr);
    }
}

/**
 * Initializes a new KD tree based on inputed point matrix.
 * There are numOfImages images, and the image with index i has numOfFeatures[i] features, or points.
 * The pointer to the point with index j of that image is in mat[i][j]. The index saved in that point is i.
 * In this function, all the pointers are saved in one long array of type SPPoint*.
 * Finally, spKDTreeInit is called with that array, to create a big kd tree.
 *
 * @param mat - the array of the arrays of pointers of the features
 * @param numOfImages - the number of images
 * @param numOfFeatures - the array containing the number of features of each image
 * @param splitMethod - the method used to determine the split dimension during the tree creation
 *
 * @return NULL in case of allocation failure occurred OR a missing point in mat OR another error in the input
 * Otherwise, the root node of the new tree is returned
 */
SPKDTreeNode* fullKDTreeCreator(SPPoint*** mat , int numOfImages, int* numOfFeatures, KD_METHOD splitMethod){
    int totalSize = 0;
    if(numOfFeatures != NULL){
        for(int i=0; i<numOfImages; i++) /* Loop to count the total number of features */
            totalSize = totalSize + numOfFeatures[i];
    }
    if(totalSize <1 || mat == NULL){
        spLoggerPrintError(ERRORMSG_INVALID_ARGS,__FILE__,__func__,__LINE__);
        return NULL;
    }
    SPPoint** allPoints = (SPPoint**) malloc(totalSize * sizeof(*allPoints));
    if(allPoints == NULL){
        spLoggerPrintError(ERRORMSG_ALLOCATION,__FILE__,__func__,__LINE__);
        return NULL;
    }
    int k = 0;
    for(int i = 0; i<numOfImages; i++){
        for(int j = 0; j < numOfFeatures[i]; j++){
            allPoints[k] = mat[i][j]; /* Adds all the features into one array */
            k = k+1;
        }
    }
    SPKDTreeNode* root = spKDTreeInit(splitMethod , allPoints, totalSize); /* Makes array into kd tree */
    free(allPoints);
    return root;
}

/**
 * Returns an array containing the indices of the spNumOfSimilarImages most similar images to the target image.
 * Pointers to the features of the target image are in the targetFeatures array. The root of the kd tree containing
 * all the features of all the images to search is root.
 *
 * A bounded priority queue, bpQueue, of size kNN is defined. For each target feature with index i:
 * bpQueue is filled with the kNN indices of the images that contain features that are closest to the target feature,
 * using the function kNearestNeighboursTree(bpQueue , root, targetFeatures[i]).
 * For each image index j in the queue, a counter for that image, imageResults[j], goes up by one.
 * If the same index appears more than once, imageResults[j] only goes up by one. The queue is then emptied.
 *
 * The spNumOfSimilarImages image indices with the highest values in imageResults are placed in a
 * sorted array, closestImages, and this array is returned.
 *
 * @param kNN - the size of the bounded priority queue
 * @param closestImages - return parameter - array containing indices of similar images found
 * @param spNumOfSimilarImages - the number of similar images to find
 * @param targetFeatures - the array containing pointers to the features of the target image
 * @param numOfTargetFeatures - the number of features the target image has
 * @param root - the root node of the kd tree containing all the features of the images to search
 * @param numOfImages - the number of images to search. All image indices will be between 0 and numOfImages-1
 *
 * @return -1 in case of allocation failure occurred OR an error in the inputed variables
 * Otherwise, 0
 */
int closestImagesSearch(int kNN, int* closestImages, int spNumOfSimilarImages, SPPoint** targetFeatures, int numOfTargetFeatures, SPKDTreeNode* root, int numOfImages){
	if(closestImages == NULL || targetFeatures == NULL || root == NULL || numOfTargetFeatures < 1 || numOfImages < 1 || kNN < 1|| spNumOfSimilarImages < 1){
		spLoggerPrintError(ERRORMSG_INVALID_ARGS,__FILE__,__func__,__LINE__);
		return -1;
	}
	int* imageResults = (int*) malloc(numOfImages * sizeof(int)); /* imageResults[i] is the number of features image i has that are close to features in targetFeatures. */
	int* imageCheck = (int*) malloc(numOfImages * sizeof(int)); /* targetFeatures[imageCheck[i]] is the last feature that was close to a feature in image i. */
	BPQueueElement* peekElementPointer = (BPQueueElement*) malloc(sizeof(*peekElementPointer)); /* Element required to check the queues */
    SPBPQueue* bpQueue = spBPQueueCreate(kNN); /* This queue will be filled with similar features, and emptied, for each feature in targetFeatures */

    if(imageResults == NULL || imageCheck == NULL || peekElementPointer == NULL || bpQueue == NULL){
        if(imageResults != NULL) free(imageResults);
        if(imageCheck != NULL) free(imageCheck);
        if(peekElementPointer != NULL) free(peekElementPointer);
        if(bpQueue != NULL) spBPQueueDestroy(bpQueue);
        return -1;
	}

    // Initialisation of closestImages
    for(int i = 1; i < spNumOfSimilarImages; i++){
        closestImages[i] = -1;
    }
    closestImages[0] = 0; // There is at least 1 image to compare, so there must be a closest image by default
    for(int i = 0; i < numOfImages; i++){
        imageResults[i] = 0; // Initialisation of imageResults
        imageCheck[i] = -1; // Initialisation of imageCheck
    }
    for(int i = 0; i < numOfTargetFeatures; i++){ // The main loop
        kNearestNeighboursTree(bpQueue , root, targetFeatures[i]); // Fill bpQueue with close features
        if(bpQueue != NULL){
            while(spBPQueueIsEmpty(bpQueue) == false){
                spBPQueuePeek(bpQueue, peekElementPointer);
                if(imageCheck[peekElementPointer->index] < i){ // This is true only if a feature in image peekElementPointer->index has not previously been found in the queue for feature targetFeatures[i]
                    imageResults[peekElementPointer->index] = imageResults[peekElementPointer->index]+1;
                    imageCheck[peekElementPointer->index] = i; // This is to avoid counting the same image twice for one feature
                }
                spBPQueueDequeue(bpQueue);
            }
        }
    }
    int numOfClosestImages = 1; // The number of closest images found, out of a possible spNumOfSimilarImages
    int nextIndex = 0;
    for(int i = 1; i < numOfImages; i++){ // During this loop, the indices of the spNumOfSimilarImages closest images will be placed in closestImages
        for(nextIndex = numOfClosestImages; nextIndex > 0 && imageResults[i] > imageResults[closestImages[nextIndex-1]];nextIndex--);
        if(nextIndex < spNumOfSimilarImages){ // True if the array is not yet full or if images i is closer than image closestImages[nextIndex]
            if(numOfClosestImages < spNumOfSimilarImages)
                numOfClosestImages = numOfClosestImages + 1;
            for(int changedIndex = numOfClosestImages-1; changedIndex>nextIndex ; changedIndex--)
                closestImages[changedIndex] = closestImages[changedIndex-1]; // The indices of images that are further away than image i need to be moved along closestPoints
            closestImages[nextIndex] = i;
        }
    }

    // Release allocated memory
    spBPQueueDestroy(bpQueue);
    free(peekElementPointer);
    free(imageResults);
    free(imageCheck);

    return 0;
}
