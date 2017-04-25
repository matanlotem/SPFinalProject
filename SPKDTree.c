#include <malloc.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "SPPoint.h"
#include "SPKDArray.h"
#include "SPKDTree.h"
#include "SPBPriorityQueue.h"
#include "SPConfig.h"

/**
 * SPKDTree Summary
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

SPKDTreeNode* spKDTreeInit(KD_METHOD splitMethod , SPPoint** pointsArray, int pointsArraySize){
	if(pointsArray == NULL || pointsArraySize < 1){
        // NULL Input error
		return NULL;
	}
	SPKDArray* kdA = spKDArrayInit(pointsArray, pointsArraySize);
	if(kdA == NULL)
		return NULL;
    return spKDTreeInitRecursion (splitMethod, kdA, 0);
}

SPKDTreeNode* spKDTreeInitRecursion(KD_METHOD splitMethod , SPKDArray* kdA, int coorSplit){
	SPKDTreeNode* newNode = (SPKDTreeNode*) malloc(sizeof(*newNode));
	if(newNode == NULL){
        // NULL allocation error
		return NULL;
	}
	if(spKDArrayGetSize(kdA) == 1){ /* Leaf initialisation */
		newNode->left = NULL;
		newNode->right = NULL;
		newNode->dim = -1;
		newNode->val = 0;
		newNode->data = (spKDArrayGetArray(kdA))[0];
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


int* closestImagesSearch(int kNN, SPPoint** targetFeatures, int numOfTargetFeatures, SPKDTreeNode* root, int numOfImages){
	if(targetFeatures == NULL || root == NULL || numOfTargetFeatures < 1 || numOfImages < 1 || kNN < 1){
        // NULL Input Error, empty array
		return NULL;
	}
	int* imageResults = (int*) malloc(numOfImages * sizeof(int)); /* imageResults[i] is the number of features image i has that are close to features in targetFeatures. */
	int* imageCheck = (int*) malloc(numOfImages * sizeof(int)); /* targetFeatures[imageCheck[i]] is the last feature that was close to a feature in image i. */
	BPQueueElement* peekElementPointer = (BPQueueElement*) malloc(sizeof(*peekElementPointer)); /* Element required to check the queues */
    SPBPQueue* bpQueue = spBPQueueCreate(kNN); /* This queue will be filled with similar features, and emptied, for each feature in targetFeatures */
	if(imageResults == NULL || imageCheck == NULL || peekElementPointer == NULL || bpQueue == NULL){
        // NULL allocation error
        if(imageResults != NULL)
            free(imageResults);
        if(imageCheck != NULL)
            free(imageCheck);
        if(peekElementPointer != NULL)
            free(peekElementPointer);
        if(bpQueue != NULL)
            spBPQueueDestroy(bpQueue);
		return NULL;
	}
    for(int i = 0; i < numOfImages; i++){
        imageResults[i] = 0; /* Initialisation of imageResults */
        imageCheck[i] = -1; /* Initialisation of imageCheck */
    }
    for(int i = 0; i < numOfTargetFeatures; i++){ /* The main loop */
        kNearestNeighboursTree(bpQueue , root, targetFeatures[i]); /* Fill bpQueue with close features */
        if(bpQueue != NULL){
            while(spBPQueueIsEmpty(bpQueue) == false){
                spBPQueuePeek(bpQueue, peekElementPointer);
                if(imageCheck[peekElementPointer->index] < i){ /* This is true only if a feature in image peekElementPointer->index has not previously been found in the queue for feature targetFeatures[i] */
                    imageResults[peekElementPointer->index] = imageResults[peekElementPointer->index]+1;
                    imageCheck[peekElementPointer->index] = i; /* This is to avoid counting the same image twice for one feature */
                }
                spBPQueueDequeue(bpQueue);
            }
        }
    }
    spBPQueueDestroy(bpQueue); /* Release allocated memory */
    free(peekElementPointer);
    free(imageResults);
	return imageCheck;
}

int kNearestNeighboursTree(SPBPQueue* bpq , SPKDTreeNode* root, SPPoint* targetPoint){
	if((root == NULL) || targetPoint == NULL || bpq == NULL){
        // NULL Input Error, empty array
		return -1;
	}
    double* lowLimit = (double*) malloc(spPointGetDimension(targetPoint) * sizeof(double)); /* These arrays are used in the recursion to mark limits */
    double* highLimit = (double*) malloc(spPointGetDimension(targetPoint) * sizeof(double));
    int* lowLimitUse = (int*) malloc(spPointGetDimension(targetPoint) * sizeof(int));
    int* highLimitUse = (int*) malloc(spPointGetDimension(targetPoint) * sizeof(int));
	if(highLimit == NULL || lowLimit == NULL || highLimitUse == NULL || lowLimitUse == NULL){
        // NULL allocation error
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

double minDistanceSquared(SPPoint* targetPoint, double* highLimit, double* lowLimit, int* highLimitUse, int* lowLimitUse){
    if(targetPoint == NULL || highLimit == NULL || lowLimit == NULL || highLimitUse == NULL || lowLimitUse == NULL){
        // NULL input error
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

SPKDTreeNode* fullKDTreeCreator(SPPoint*** mat , int numOfImages, int* numOfFeatures, KD_METHOD splitMethod){
    int totalSize = 0;
    if(numOfFeatures != NULL){
        for(int i=0; i<numOfImages; i++) /* Loop to count the total number of features */
            totalSize = totalSize + numOfFeatures[i];
    }
    if(totalSize <1 || mat == NULL){
        //NULL input error
        return NULL;
    }
    SPPoint** allPoints = (SPPoint**) malloc(totalSize * sizeof(*allPoints));
    if(allPoints == NULL){
        //NULL allocation error
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
 * Frees all allocated memory of kdTree.
 *
 */
void spKDTreeDestroy(SPKDTreeNode* curr){
    if(curr != NULL){
        spKDTreeDestroy(curr->left);
        spKDTreeDestroy(curr->right);
        free(curr);
    }
}

// Useless except for checking:
SPBPQueue* kNearestNeighbours(KD_METHOD splitMethod, int kNN , SPPoint** pointsArray, int pointsArraySize, SPPoint* targetPoint){
	if(pointsArray == NULL || targetPoint == NULL || pointsArraySize < 1){
        //NULL input error
		return NULL;
	}
    SPKDTreeNode* root = spKDTreeInit(splitMethod , pointsArray, pointsArraySize);
	if(root == NULL){
		return NULL; /* The error is specified in spKDTreeInit */
	}
	SPBPQueue* bpq = spBPQueueCreate(kNN);
    kNearestNeighboursTree(bpq , root, targetPoint);
    spKDTreeDestroy(root);
	return bpq;
}

