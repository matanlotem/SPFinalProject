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
	SPPoint* data;
	SPKDTreeNode* left;
	SPKDTreeNode* right;
	int dim;
	double val;
};


SPKDTreeNode* spKDTreeInit(const SPConfig configData , SPPoint** pointsArray, int pointsArraySize){
	if(pointsArray == NULL || pointsArraySize < 1)
		return NULL;
	SPKDArray* kdA = spKDArrayInit(pointsArray, pointsArraySize);
	if(kdA == NULL)
		return NULL;
    return spKDTreeInitRecursion(configData, kdA, 0);
}

SPKDTreeNode* spKDTreeInitRecursion(const SPConfig configData , SPKDArray* kdA, int coorSplit){
    SP_CONFIG_MSG* msg = (SP_CONFIG_MSG*) malloc(sizeof(*msg));
	SPKDTreeNode* newNode = (SPKDTreeNode*) malloc(sizeof(*newNode));
	if(newNode == NULL)
		return NULL;
	if(spKDArrayGetSize(kdA) == 1){
		newNode->left = NULL;
		newNode->right = NULL;
		newNode->dim = -1;
		newNode->val = 0;
		newNode->data = (spKDArrayGetArray(kdA))[0];
	}
	else{
		int n = spKDArrayGetSize(kdA);
		int d = spKDArrayGetDimension(kdA);
		int i = 0;
		double maxSpread = 0;
		double currentSpread = 0;
		if(spConfigGetKDSplitMethod(configData, msg) == INCREMENTAL)
		{
			coorSplit = coorSplit+1;
			if(coorSplit > d)
				coorSplit = 1;
		}
		if(spConfigGetKDSplitMethod(configData, msg)  == RANDOM)
		{
			coorSplit = (rand() % d) + 1;
		}
		if(spConfigGetKDSplitMethod(configData, msg)  == MAX_SPREAD)
		{
			coorSplit = 1;
			for(i = 0; i<d; i++){
				currentSpread = spPointGetAxisCoor((spKDArrayGetArray(kdA))[(spKDArrayGetIndicesByDim(kdA, i+1))[n-1]],i) - spPointGetAxisCoor((spKDArrayGetArray(kdA))[(spKDArrayGetIndicesByDim(kdA, i+1))[0]],i);
				if(maxSpread < currentSpread){
					maxSpread = currentSpread;
					coorSplit = i+1;
				}
			}
		}
		SPKDArray** kdASplit = spKDArraySplit(kdA, coorSplit);
		newNode->dim = coorSplit;
		i = n;
		if(n % 2 == 1)
			i = i-1;
		i = (int)(i/2);
		newNode->val = spPointGetAxisCoor((spKDArrayGetArray(kdA))[(spKDArrayGetIndicesByDim(kdA, coorSplit))[i]],coorSplit-1);
		newNode->data = NULL;
		newNode->left = spKDTreeInitRecursion(configData, kdASplit[0], coorSplit);
		newNode->right = spKDTreeInitRecursion(configData, kdASplit[1], coorSplit);
		free(kdASplit);
		spKDArrayDestroy(kdA);
	}
	free(msg);
    return newNode;
}


SPBPQueue* kNearestNeighbours(const SPConfig configData , SPPoint** pointsArray, int pointsArraySize, SPPoint* targetPoint){
	if( (pointsArray == NULL || targetPoint == NULL) || pointsArraySize < 1)
		return NULL;
    SPKDTreeNode* root = spKDTreeInit(configData , pointsArray, pointsArraySize);
	if(root == NULL)
		return NULL;
    SPBPQueue* bpq = kNearestNeighboursTree(configData , pointsArray, root, targetPoint);
    spKDTreeDestroy(root);
	return bpq;
}

SPBPQueue* kNearestNeighboursTree(const SPConfig configData , SPPoint** pointsArray, SPKDTreeNode* root, SPPoint* targetPoint){
    SP_CONFIG_MSG* msg = (SP_CONFIG_MSG*) malloc(sizeof(*msg));
	if((root == NULL || pointsArray == NULL) || targetPoint == NULL)
		return NULL;
    SPBPQueue* bpq = spBPQueueCreate(spConfigGetKNN(configData, msg));
    double* lowLimit = (double*) malloc(spPointGetDimension(targetPoint) * sizeof(double));
    double* highLimit = (double*) malloc(spPointGetDimension(targetPoint) * sizeof(double));
    int* lowLimitUse = (int*) malloc(spPointGetDimension(targetPoint) * sizeof(int));
    int* highLimitUse = (int*) malloc(spPointGetDimension(targetPoint) * sizeof(int));
	if(bpq == NULL || ((highLimit == NULL || lowLimit == NULL) || (highLimitUse == NULL || lowLimitUse == NULL))){
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
        return NULL;
	}
	int i = 0;
    for(i = 0; i<spPointGetDimension(targetPoint) ; i++){
        highLimit[i] = 0;
        lowLimit[i] = 0;
        highLimitUse[i] = 0;
        lowLimitUse[i] = 0;
    }
    kNearestNeighboursRecursion(bpq, pointsArray, root, targetPoint, highLimit, lowLimit, highLimitUse, lowLimitUse);
    free(highLimit);
    free(lowLimit);
    free(highLimitUse);
    free(lowLimitUse);
    free(msg);
	return bpq;
}

void kNearestNeighboursRecursion(SPBPQueue* bpq, SPPoint** pointsArray, SPKDTreeNode* curr, SPPoint* targetPoint, double* highLimit, double* lowLimit, int* highLimitUse, int* lowLimitUse){
    if(curr != NULL){
        if(isLeaf(curr)){
            spBPQueueEnqueue(bpq, spPointGetIndex(getData(curr)), spPointL2SquaredDistance(targetPoint, getData(curr)));
        }
        else{
            int cont = 1;
            int currentDimIndex = curr->dim -1;
            double currentLowLimit = lowLimit[currentDimIndex];
            double currentHighLimit = highLimit[currentDimIndex];
            int currentLowLimitUse = lowLimitUse[currentDimIndex];
            int currentHighLimitUse = highLimitUse[currentDimIndex];

            highLimit[currentDimIndex] = curr->val;
            highLimitUse[currentDimIndex] = 1;
            if(spBPQueueIsFull(bpq) == true){
                if(minDistanceSquared(targetPoint, highLimit, lowLimit, highLimitUse, lowLimitUse) >= spBPQueueMaxValue(bpq))
                    cont = 0;
            }
            if(cont == 1){
                kNearestNeighboursRecursion(bpq, pointsArray, curr->left, targetPoint, highLimit, lowLimit, highLimitUse, lowLimitUse);
            }
            highLimit[currentDimIndex] = currentHighLimit;
            highLimitUse[currentDimIndex] = currentHighLimitUse;

            lowLimit[currentDimIndex] = curr->val;
            lowLimitUse[currentDimIndex] = 1;
            if(spBPQueueIsFull(bpq) == true){
                if(minDistanceSquared(targetPoint, highLimit, lowLimit, highLimitUse, lowLimitUse) >= spBPQueueMaxValue(bpq))
                    cont = 0;
            }
            if(cont == 1){
                kNearestNeighboursRecursion(bpq, pointsArray, curr->right, targetPoint, highLimit, lowLimit, highLimitUse, lowLimitUse);
            }
            lowLimit[currentDimIndex] = currentLowLimit;
            lowLimitUse[currentDimIndex] = currentLowLimitUse;
        }
    }
}

double minDistanceSquared(SPPoint* targetPoint, double* highLimit, double* lowLimit, int* highLimitUse, int* lowLimitUse){
    double* closestPointCoor = (double*) malloc(spPointGetDimension(targetPoint) * sizeof(double));
    int i = 0;
    for(i = 0; i<spPointGetDimension(targetPoint) ; i++){
        closestPointCoor[i]=spPointGetAxisCoor(targetPoint, i);
        if(lowLimitUse[i] == 1){
            if(closestPointCoor[i] < lowLimit[i])
                closestPointCoor[i]=lowLimit[i];
        }
        if(highLimitUse[i] == 1){
            if(closestPointCoor[i] > highLimit[i])
                closestPointCoor[i]=highLimit[i];
        }
    }
    SPPoint* closestPoint =spPointCreate(closestPointCoor, spPointGetDimension(targetPoint) , 1);
    double res = spPointL2SquaredDistance(targetPoint, closestPoint);
    free(closestPointCoor);
    spPointDestroy(closestPoint);
    return res;
}

bool isLeaf(SPKDTreeNode* curr){
	if(curr == NULL)
		return false;
	if(curr->data == NULL)
		return false;
	return true;
}

SPPoint* getData(SPKDTreeNode* curr){
	if(curr == NULL)
		return NULL;
	return curr->data;
}

SPKDTreeNode* getLeft(SPKDTreeNode* curr){
	if(curr == NULL)
		return NULL;
	return curr->left;
}

SPKDTreeNode* getRight(SPKDTreeNode* curr){
	if(curr == NULL)
		return NULL;
	return curr->right;
}

int getSplitDimension(SPKDTreeNode* curr){
	if(curr == NULL)
		return -2;
	return curr->dim;
}

double getMedianValue(SPKDTreeNode* curr){
	if(curr == NULL)
		return 0;
	return curr->val;
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



SPKDTreeNode* spKDTreeInitNoConfig(int splitMethod , SPPoint** pointsArray, int pointsArraySize){
	if(pointsArray == NULL || pointsArraySize < 1)
		return NULL;
	SPKDArray* kdA = spKDArrayInit(pointsArray, pointsArraySize);
	if(kdA == NULL)
		return NULL;
    return spKDTreeInitRecursionNoConfig(splitMethod, kdA, 0);
}

SPKDTreeNode* spKDTreeInitRecursionNoConfig(int splitMethod , SPKDArray* kdA, int coorSplit){
	SPKDTreeNode* newNode = (SPKDTreeNode*) malloc(sizeof(*newNode));
	if(newNode == NULL)
		return NULL;
	if(spKDArrayGetSize(kdA) == 1){
		newNode->left = NULL;
		newNode->right = NULL;
		newNode->dim = -1;
		newNode->val = 0;
		newNode->data = (spKDArrayGetArray(kdA))[0];
	}
	else{
		int n = spKDArrayGetSize(kdA);
		int d = spKDArrayGetDimension(kdA);
		int i = 0;
		double maxSpread = 0;
		double currentSpread = 0;
		if(splitMethod == 2) // INCREMENTAL
		{
			coorSplit = coorSplit+1;
			if(coorSplit > d)
				coorSplit = 1;
		}
		if(splitMethod == 0) // RANDOM
		{
			coorSplit = (rand() % d) + 1;
		}
		if(splitMethod == 1) // MAX_SPREAD
		{
			coorSplit = 1;
			for(i = 0; i<d; i++){
				currentSpread = spPointGetAxisCoor((spKDArrayGetArray(kdA))[(spKDArrayGetIndicesByDim(kdA, i+1))[n-1]],i) - spPointGetAxisCoor((spKDArrayGetArray(kdA))[(spKDArrayGetIndicesByDim(kdA, i+1))[0]],i);
				if(maxSpread < currentSpread){
					maxSpread = currentSpread;
					coorSplit = i+1;
				}
			}
		}
		SPKDArray** kdASplit = spKDArraySplit(kdA, coorSplit);
		newNode->dim = coorSplit;
		i = n;
		if(n % 2 == 1)
			i = i-1;
		i = (int)(i/2);
		newNode->val = spPointGetAxisCoor((spKDArrayGetArray(kdA))[(spKDArrayGetIndicesByDim(kdA, coorSplit))[i]],coorSplit-1);
		newNode->data = NULL;
		newNode->left = spKDTreeInitRecursionNoConfig(splitMethod, kdASplit[0], coorSplit);
		newNode->right = spKDTreeInitRecursionNoConfig(splitMethod, kdASplit[1], coorSplit);
		free(kdASplit);
		spKDArrayDestroy(kdA);
	}
    return newNode;
}


SPBPQueue* kNearestNeighboursNoConfig(int splitMethod, int kNN , SPPoint** pointsArray, int pointsArraySize, SPPoint* targetPoint){
	if( (pointsArray == NULL || targetPoint == NULL) || pointsArraySize < 1)
		return NULL;
    SPKDTreeNode* root = spKDTreeInitNoConfig(splitMethod , pointsArray, pointsArraySize);
	if(root == NULL)
		return NULL;
    SPBPQueue* bpq = kNearestNeighboursTreeNoConfig(kNN , pointsArray, root, targetPoint);
    spKDTreeDestroy(root);
	return bpq;
}

SPBPQueue* kNearestNeighboursTreeNoConfig(int kNN , SPPoint** pointsArray, SPKDTreeNode* root, SPPoint* targetPoint){
	if((root == NULL || pointsArray == NULL) || targetPoint == NULL)
		return NULL;
    SPBPQueue* bpq = spBPQueueCreate(kNN);
    double* lowLimit = (double*) malloc(spPointGetDimension(targetPoint) * sizeof(double));
    double* highLimit = (double*) malloc(spPointGetDimension(targetPoint) * sizeof(double));
    int* lowLimitUse = (int*) malloc(spPointGetDimension(targetPoint) * sizeof(int));
    int* highLimitUse = (int*) malloc(spPointGetDimension(targetPoint) * sizeof(int));
	if(bpq == NULL || ((highLimit == NULL || lowLimit == NULL) || (highLimitUse == NULL || lowLimitUse == NULL))){
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
        return NULL;
	}
	int i = 0;
    for(i = 0; i<spPointGetDimension(targetPoint) ; i++){
        highLimit[i] = 0;
        lowLimit[i] = 0;
        highLimitUse[i] = 0;
        lowLimitUse[i] = 0;
    }
    kNearestNeighboursRecursion(bpq, pointsArray, root, targetPoint, highLimit, lowLimit, highLimitUse, lowLimitUse);
    free(highLimit);
    free(lowLimit);
    free(highLimitUse);
    free(lowLimitUse);
	return bpq;
}

