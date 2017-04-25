#ifndef SPKDTREE_H_INCLUDED
#define SPKDTREE_H_INCLUDED
#include "SPKDArray.h"
#include "SPConfig.h"
#include "SPBPriorityQueue.h"

/**
 * SPKDTree Summary
 *
 */

/** Type for defining the tree node **/
typedef struct kd_tree_node_t SPKDTreeNode;


/**
 */
SPKDTreeNode* spKDTreeInit(KD_METHOD splitMethod , SPPoint** pointsArray, int pointsArraySize);

SPKDTreeNode* spKDTreeInitRecursion(KD_METHOD splitMethod , SPKDArray* kdA, int coorSplit);

int kNearestNeighboursTree(SPBPQueue* bpq , SPKDTreeNode* root, SPPoint* targetPoint);

void kNearestNeighboursRecursion(SPBPQueue* bpq, SPKDTreeNode* curr, SPPoint* targetPoint, double* highLimit, double* lowLimit, int* highLimitUse, int* lowLimitUse);

double minDistanceSquared(SPPoint* targetPoint, double* highLimit, double* lowLimit, int* highLimitUse, int* lowLimitUse);

/**
 * Frees all allocated memory of kdTree.
 *
 */
void spKDTreeDestroy(SPKDTreeNode* curr);

SPKDTreeNode* fullKDTreeCreator(SPPoint*** mat , int numOfImages, int* numOfFeatures, KD_METHOD splitMethod);

int* closestImagesSearch(int kNN, int spNumOfSimilarImages, SPPoint** targetFeatures, int numOfTargetFeatures, SPKDTreeNode* root, int numOfImages);

SPBPQueue* kNearestNeighbours(KD_METHOD splitMethod, int kNN , SPPoint** pointsArray, int pointsArraySize, SPPoint* targetPoint);

#endif // SPKDTREE_H_INCLUDED
