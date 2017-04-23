#include <malloc.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "SPPoint.h"
#include "SPKDArray.h"
#include "SPKDTree.h"
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


SPKDTreeNode* spKDTreeInit(SPPoint** arr, int n){
	if(arr == NULL || n < 1)
		return NULL;
	SPKDArray* kdA = spKDArrayInit(arr, n);
	if(kdA == NULL)
		return NULL;
    return spKDTreeInitRecursion(kdA, 0);
}

SPKDTreeNode* spKDTreeInitRecursion(SPKDArray* kdA, int coorSplit){
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
		//if(INCREMENTAL)
		{
			coorSplit = coorSplit+1;
			if(coorSplit > d)
				coorSplit = 1;
		}
		//if(RANDOM)
		{
			coorSplit = (rand() % d) + 1;
		}
		//if(MAX_SPREAD)
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
		newNode->left = spKDTreeInitRecursion(kdASplit[0], coorSplit);
		newNode->right = spKDTreeInitRecursion(kdASplit[1], coorSplit);
		free(kdASplit);
		spKDArrayDestroy(kdA);
	}
    return newNode;
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

