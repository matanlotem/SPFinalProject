#ifndef SPKDTREE_H_INCLUDED
#define SPKDTREE_H_INCLUDED
#include "SPKDArray.h"
#include "SPConfig.h"
#include "SPBPriorityQueue.h"

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
typedef struct kd_tree_node_t SPKDTreeNode;

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
SPKDTreeNode* spKDTreeInit(KD_METHOD splitMethod , SPPoint** pointsArray, int pointsArraySize);

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
SPKDTreeNode* spKDTreeInitRecursion(KD_METHOD splitMethod , SPKDArray* kdA, int coorSplit);

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
int kNearestNeighboursTree(SPBPQueue* bpq , SPKDTreeNode* root, SPPoint* targetPoint);

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
void kNearestNeighboursRecursion(SPBPQueue* bpq, SPKDTreeNode* curr, SPPoint* targetPoint, double* highLimit, double* lowLimit, int* highLimitUse, int* lowLimitUse);

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
double minDistanceSquared(SPPoint* targetPoint, double* highLimit, double* lowLimit, int* highLimitUse, int* lowLimitUse);

/**
 * Frees all allocated memory of kd tree. It is used recursively on each child.
 *
 * @param curr - the tree node to free
 */
void spKDTreeDestroy(SPKDTreeNode* curr);

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
SPKDTreeNode* fullKDTreeCreator(SPPoint*** mat , int numOfImages, int* numOfFeatures, KD_METHOD splitMethod);

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
 * @param spNumOfSimilarImages - the number of similar images to find
 * @param targetFeatures - the array containing pointers to the features of the target image
 * @param numOfTargetFeatures - the number of features the target image has
 * @param root - the root node of the kd tree containing all the features of the images to search
 * @param numOfImages - the number of images to search. All image indices will be between 0 and numOfImages-1
 *
 * @return NULL in case of allocation failure occurred OR an error in the inputed variables
 * Otherwise, the array of closest image indices is returned
 */
int* closestImagesSearch(int kNN, int spNumOfSimilarImages, SPPoint** targetFeatures, int numOfTargetFeatures, SPKDTreeNode* root, int numOfImages);

/* Used in checking, not necessary */
SPBPQueue* kNearestNeighbours(KD_METHOD splitMethod, int kNN , SPPoint** pointsArray, int pointsArraySize, SPPoint* targetPoint);

#endif // SPKDTREE_H_INCLUDED
