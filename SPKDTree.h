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
SPKDTreeNode* spKDTreeInit(SPConfig configData , SPPoint** arr, int n);

SPKDTreeNode* spKDTreeInitRecursion(SPConfig configData , SPKDArray* kdA, int coorSplit);

SPBPQueue* kNearestNeighbours(SPConfig configData , SPPoint** pointsArray, int pointsArraySize, SPPoint* targetPoint);

SPBPQueue* kNearestNeighboursTree(SPConfig configData , SPPoint** pointsArray, SPKDTreeNode* root, SPPoint* targetPoint);

void kNearestNeighboursRecursion(SPBPQueue* bpq, SPPoint** pointsArray, SPKDTreeNode* curr, SPPoint* targetPoint, double* highLimit, double* lowLimit, int* highLimitUse, int* lowLimitUse);

double minDistanceSquared(SPPoint* targetPoint, double* highLimit, double* lowLimit, int* highLimitUse, int* lowLimitUse);

bool isLeaf(SPKDTreeNode* curr);

SPPoint* getData(SPKDTreeNode* curr);

SPKDTreeNode* getLeft(SPKDTreeNode* curr);

SPKDTreeNode* getRight(SPKDTreeNode* curr);

int getSplitDimension(SPKDTreeNode* curr);

double getMedianValue(SPKDTreeNode* curr);

/**
 * Frees all allocated memory of kdTree.
 *
 */
void spKDTreeDestroy(SPKDTreeNode* curr);


SPKDTreeNode* spKDTreeInitNoConfig(int splitMethod , SPPoint** pointsArray, int pointsArraySize);

SPKDTreeNode* spKDTreeInitRecursionNoConfig(int splitMethod , SPKDArray* kdA, int coorSplit);

SPBPQueue* kNearestNeighboursNoConfig(int splitMethod, int kNN , SPPoint** pointsArray, int pointsArraySize, SPPoint* targetPoint);

SPBPQueue* kNearestNeighboursTreeNoConfig(int kNN , SPPoint** pointsArray, SPKDTreeNode* root, SPPoint* targetPoint);

#endif // SPKDTREE_H_INCLUDED
