#ifndef SPKDARRAY_H_INCLUDED
#define SPKDARRAY_H_INCLUDED

/**
 * SPKDArray Summary
 * Encapsulates an array of points, each having the same dimension.
 * The points are ordered differently by each dimension, and each
 * possible order is saved in the KD Array as an array.
 *
 * The following functions are supported:
 *
 * spKDArrayInit            	- Initializes a KD array based on an array of points.
 * spKDArraySplit 		    	- Splits the KD array into 2 based on the ordering by a given dimension.
 * spCopyPointArray		    	- Create a new copy of the pointers in a given point array.
 * spSortPointArrayByDimension	- Orders an array of indices by a given dimension.
 * spKDArrayGetDimension		- A getter of the dimension of all the points in the KD array.
 * spKDArrayGetSize     		- A getter of the number of points in the KD array.
 * spKDArrayGetArray    		- A getter of the array of points.
 * spKDArrayGetIndicesByDim 	- A getter of an array of indices as sorted by a given dimension.
 * spKDArrayDestroy     		- Frees all allocated memory in the KD Array.
 *
 */

/** Type for defining the array **/
typedef struct kd_array_t SPKDArray;

/**
 * Initializes a new KD array based on inputed point array and size of array.
 * If d is the dimension of each point, and there are n points,
 * a matrix of d rows and n columns is created, each cell holding
 * an index of an SPPoint in the array.
 * Each row is sorted by the coordinates of the points in the row number dimension.
 *
 * @param arr - the array of pointers to points
 * @param size - the number of pointers to points
 *
 * @return NULL in case allocation failure occurred OR arr is NULL OR not all the dimensions are the same
 * Otherwise, the new KD Array is returned
 */
SPKDArray* spKDArrayInit(SPPoint** arr, int size);

/**
 * Initializes a new KD array using the sorted matrix given as input.
 * Requires the matrix to be sorted correctly in each row.
 *
 * @param data - the array of pointers to points
 * @param a - the sorted matrix of indices
 * @param size - the number of pointers to points
 * @param d - the dimensions of each point
 *
 * @return The new KD Array is returned (NULL in case of allocation error)
 */
SPKDArray* spKDArrayInitPreSorted(SPPoint** data, int** a, int size , int d);

/**
 * Splits an inputed KD array in half, into 2 KD arrays.
 * The points are sorted by their coordnates in the inputed dimension, coor,
 * with the points with the lower coordinates in that dimension in one KD array,
 * and the rest in the other.
 *
 * @param kdArr - the kd array to split
 * @param coor - the dimension to split by (a number from 1 to d, not the index from 0 to d-1)
 *
 * @return The output is an array of 2 KD Array pointers, the first being kdLeft and the second kdRight.
 * NULL is returned in case of allocation error.
 */
SPKDArray** spKDArraySplit(SPKDArray* kdArr, int coor);

/**
 * Makes a new copy of an SPPoint pointers array, base, with size elements.
 *
 * @param base - the array of pointers to points to copy
 * @param size - the number of pointers to points
 *
 * @return The copy is returned (NULL in case of allocation error)
 */
SPPoint** spCopyPointArray(SPPoint** base, int size );

/**
 * Merge sorts row number d of the inputed matrix a by the coordinates of the points in the inputed dimension.
 * The sort is logarithmic, complexity O(size log(size)). It requires an int array, tempArray, with size elements
 * for temporary sorting purposes, since it would be less efficient to reallocate memory to one each time sort is called.
 *
 * @param a - the matrix of indices
 * @param data - the array of pointers to points
 * @param size - the number of pointers to points (number of columns in a)
 * @param d - the index of the row to sort in a
 * @param tempArray - a temporary array with the same size as the row in a
 *
 * @return 1 if sort succeeded (-1 in case of error)
 */
int spSortPointArrayByDimension(int** a , SPPoint** data, int size, int d , int* tempArray);

/**
 * Returns the dimension of the points in the KD array.
 *
 * @param kdA - the kd array
 *
 * @return The output is dim.
 */
int spKDArrayGetDimension(SPKDArray* kdA);

/**
 * Returns the number of points in the KD array.
 *
 * @param kdA - the kd array
 *
 * @return The output is size.
 */
int spKDArrayGetSize(SPKDArray* kdA);

/**
 * Returns the array of points in the KD array.
 *
 * @param kdA - the kd array
 *
 * @return The output is arr.
 */
SPPoint** spKDArrayGetArray(SPKDArray* kdA);

/**
 * Returns the indices of the array of points, a sorted by their dim coordinate.
 *
 * @param kdA - the kd array
 * @param dim - the dimension number, from 1 to d
 *
 * @return The output is arrIndices[dim-1].
 */
int* spKDArrayGetIndicesByDim(SPKDArray* kdA, int dim);

/**
 * Frees all allocated memory of kdA.
 *
 * @param kdA - the kd array
 *
 */
void spKDArrayDestroy(SPKDArray* kdA);

#endif // SPKDARRAY_H_INCLUDED
