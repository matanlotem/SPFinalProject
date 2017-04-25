#include <malloc.h>
#include <assert.h>
#include "SPPoint.h"
#include "SPKDArray.h"

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
 * spCopyPointArray		    	- Create a new copy of a given point array.
 * spSortPointArrayByDimension	- Orders an array of indices by a given dimension.
 * spKDArrayGetDimension		- A getter of the dimension of all the points in the KD array.
 * spKDArrayGetSize     		- A getter of the number of points in the KD array.
 * spKDArrayGetArray    		- A getter of the array of points.
 * spKDArrayGetIndicesByDim 	- A getter of an array of indices as sorted by a given dimension.
 * spKDArrayDestroy     		- Frees all allocated memory in the KD Array.
 *
 */

/** Type for defining the array **/
struct kd_array_t {
	SPPoint** arr; /* The array of pointers to the points  */
	int** arrIndices; /* Each row contains the sorted indices for the dimension with that row number */
	int dim; /* The number of dimensions each point has */
	int size; /* The number of points */
};

/**
 * Initializes a new KD array based on inputed point array and size of array.
 * If d is the dimension of each point, and there are n points,
 * a matrix of d rows and n columns is created, each cell holding
 * an index of an SPPoint in the array.
 * Each row is sorted by the coordinates of the points in the row number dimension.
 *
 * @return
 * NULL in case allocation failure occurred OR arr is NULL OR not all the dimensions are the same
 * Otherwise, the new KD Array is returned
 */
SPKDArray* spKDArrayInit(SPPoint** arr, int size){
    int iError = -1; /* Allocation error checker */
    int d = 0; /* Number of dimensions of each point */
    if(size>0 && arr != NULL){
        for(int i = 0; i<size ; i++){ /* Check that all points exist and have the same dimension */
            if(arr[i] == NULL){
                d = 0;
                i = size;
            }
            else if(i == 0)
                d = spPointGetDimension((arr[i]));
            else if(d != spPointGetDimension((arr[i]))){
                d = -1;
                i = size;
            }
        }
        if(d > 0){
            int **a = (int**) malloc(d*sizeof(*a)); /* a is the sorted matrix of indexes */
            if(a == NULL)
                // NULL allocation error
                return NULL;
            int* tempArray = (int*) malloc(size*sizeof(int)); /* tempArray will help in sort */
            if(tempArray == NULL){
                // NULL allocation error
                free(a);
                return NULL;
            }
            for(int i = 0; i<d ; i++){
                a[i] = (int*) malloc(size*sizeof(int));
                if(a[i] == NULL){
                    d = -1;
                    iError = i;
                }
                else{
                    for(int j = 0; j<size; j++)
                        a[i][j] = j; /* The initial index order before sort is 0,1,...,size-1 in each row */
                    spSortPointArrayByDimension(a , arr , size, i, tempArray); /* Merge sort of row i */
                }
            }
            free(tempArray);
            if(d == -1){
                // NULL allocation error
                for(int i = 0; i < iError ; i++){
                    free(a[i]);
                }
                free(a);
                return NULL;
            }
            SPPoint** dataCopy = spCopyPointArray(arr , size);
            if(dataCopy == NULL){
                // NULL allocation error
                for(int i = 0; i<d ; i++){
                    free(a[i]);
                }
                free(a);
                return NULL;
            }
            SPKDArray* res = spKDArrayInitPreSorted(dataCopy, a, size , d);
            if(res == NULL)
            {
                // NULL allocation error
                for(int i = 0; i<d ; i++)
                    free(a[i]);
                free(a);
                free(dataCopy);
                return NULL;
            }
            return res;
        }
    }
    // NULL input error (NULL array or some points have different dimensions than others)
    return NULL;
}

/**
 * Initializes a new KD array using the sorted matrix given as input.
 * Requires the matrix to be sorted correctly in each row.
 *
 * @return
 * The new KD Array is returned (NULL in case of allocation error)
 */
SPKDArray* spKDArrayInitPreSorted(SPPoint** data, int** a, int size , int d){
    SPKDArray *res = (SPKDArray*) malloc(sizeof(*res));
    if(res != NULL){
        res->dim = d;
        res->size = size;
        res->arr = data;
        res->arrIndices = a;
        return res;
    }
    return NULL; /* This error is handled in above function */

}


/**
 * Splits an inputed KD array in half, into 2 KD arrays.
 * The points are sorted by their coordnates in the inputed dimension, coor,
 * with the points with the lower coordinates in that dimension in one KD array,
 * and the rest in the other.
 *
 * @return
 * The output is an array of 2 KD Array pointers, the first being kdLeft and the second kdRight.
 * NULL is returned in case of allocation error.
 */
SPKDArray** spKDArraySplit(SPKDArray* kdArr, int coor){
    if(kdArr == NULL){
        // NULL input error
        return NULL;
    }
    if((coor < 1 || coor > kdArr->dim) || kdArr->size < 2){
        // NULL input error
        return NULL;
    }
    int n1 = kdArr->size;
    if(n1 % 2 == 1)
        n1 = n1+1;
    n1 = (int) n1/2; /* The number of points of the left array */
    int n2 = kdArr->size - n1; /* The number of points of the right array */
    int kError = 0; /* Allocation error checker */
    int j1 = 0; /* Index for left array */
    int j2 = 0; /* Index for right array */
    SPKDArray** res = (SPKDArray**) malloc(2*(sizeof(kdArr))); /* res[0] is the pointer to the left array, and res[1] is the pointer to the right array */
    int* tempSplitArray = (int*) malloc((kdArr->size)*sizeof(int)); /* tempSplitArray[i] is the index of point i in row kdArr->arrIndices[coor-1] */
    int* tempNewIndex = (int*) malloc((kdArr->size)*sizeof(int)); /* tempNewIndex[i] is the index of point i in the new left or right array */
    SPPoint **dataLeft = (SPPoint**) malloc(n1*sizeof(*dataLeft)); /* The points of the left array */
    SPPoint **dataRight = (SPPoint**) malloc(n2*sizeof(*dataRight)); /* The points of the right array */
    int **aLeft = (int**) malloc((kdArr->dim)*sizeof(*aLeft)); /* The matrix of indices of the left array */
    int **aRight = (int**) malloc((kdArr->dim)*sizeof(*aRight)); /* The matrix of indices of the right array */
    if(aRight != NULL && aLeft != NULL){
        for(int k = 0; k< kdArr->dim ; k++){ /* Loop to initialise all rows in the matrixes */
            aLeft[k] = (int*) malloc((n1)*sizeof(int));
            if(aLeft[k] == NULL){
                kError = k;
                k = k+2*kdArr->dim;
            }
            else{
                aRight[k] = (int*) malloc((n2)*sizeof(int));
                if(aRight[k] == NULL){
                    free(aLeft[k]);
                    kError = k;
                    k = k+2*kdArr->dim;
                }
            }
        }
        if(kError != 0){
            for(int k = 0; k< kError ; k++){
                free(aLeft[k]);
                free(aRight[k]);
            }
        }
    }
    if((((dataRight == NULL || dataLeft == NULL) || (res == NULL || tempSplitArray == NULL)) || (aRight == NULL || aLeft == NULL)) || (tempNewIndex == NULL || kError != 0)){
        // NULL allocation error
        if(res != NULL)
            free(res);
        if(tempNewIndex != NULL)
            free(tempNewIndex);
        if(tempSplitArray != NULL)
            free(tempSplitArray);
        if(dataLeft != NULL)
            free(dataLeft);
        if(dataRight != NULL)
            free(dataRight);
        if(aLeft != NULL)
            free(aLeft);
        if(aRight != NULL)
            free(aRight);
        return NULL;
    }
    for(int i = 0; i< kdArr->size ; i++){
        tempSplitArray[kdArr->arrIndices[coor-1][i]] = i; /* Setting the correct tempSplitArray values */
    }
    for(int i = 0; i< kdArr->size ; i++){
        if(tempSplitArray[i]<n1){
            dataLeft[j1] = kdArr->arr[i]; /* Filling the point array */
            tempNewIndex[i] = j1; /* Setting the correct tempNewIndex values */
            j1++;
        }
        else{
            dataRight[j2] = kdArr->arr[i]; /* Filling the point array */
            tempNewIndex[i] = j2; /* Setting the correct tempNewIndex values */
            j2++;
        }
    }
    for(int k = 0; k< kdArr->dim ; k++){
        j1 = 0;
        j2 = 0;
        for(int i = 0; i< kdArr->size ; i++){
            if(tempSplitArray[kdArr->arrIndices[k][i]]<n1){
                aLeft[k][j1] = tempNewIndex[kdArr->arrIndices[k][i]]; /* Filling the matrix in correct order */
                j1++;
            }
            else{
                aRight[k][j2] = tempNewIndex[kdArr->arrIndices[k][i]]; /* Filling the matrix in correct order */
                j2++;
            }
        }
    }
    free(tempSplitArray);
    free(tempNewIndex);
    res[0] = spKDArrayInitPreSorted(dataLeft, aLeft, n1 , kdArr->dim);
    res[1] = spKDArrayInitPreSorted(dataRight, aRight, n2 , kdArr->dim);
    return res;
}

/**
 * Makes a new copy of an SPPoint pointers array, base, with size elements.
 *
 * @return
 * The copy is returned (NULL in case of allocation error)
 */
SPPoint** spCopyPointArray(SPPoint** base, int size ){
    int iChecker=size;
    if(size>0 && base != NULL){
        SPPoint **res = (SPPoint**) malloc(size*(sizeof(*res)));
        if(res != NULL){
            for(int i = 0; i < size; i++){
                res[i] = NULL;
                if(base[i] != NULL)
                    res[i] = base[i];
                if(res[i] == NULL){
                    i = size+1;
                    iChecker = size+1;
                }
            }
            if(iChecker != size){
                // NULL input error (missing point)
                free(res);
                return NULL;
            }
            return res;
        }
        // NULL allocation error
        return NULL;
    }
    // NULL input error
    return NULL;
}

/**
 * Merge sorts row number d of the inputed matrix a by the coordinates of the points in the inputed dimension.
 * The sort is logarithmic, complexity O(size log(size)). It requires an int array, tempArray, with size elements
 * for temporary sorting purposes, since it would be less efficient to reallocate memory to one each time sort is called.
 *
 * @return
 * 1 if sort succeeded (-1 in case of error)
 */
int spSortPointArrayByDimension(int** a , SPPoint** data, int size, int d , int* tempArray){
    int i3 = 0; /* Counter for first group to merge */
    int i4 = 0; /* Counter for second group to merge */
    int i5 = 0; /* Counter for the result of the merge */
    int dim = 0; /* The number of dimensions in each point */
    if((size>0 && d>-1) && (a != NULL && data != NULL)){
        for(int i = 0; i < size; i++){
            if(data[i] != NULL){
                if(i == 0)
                    dim = spPointGetDimension((data[i]));
                if(spPointGetDimension((data[i])) != dim){
                    // NULL input error (wrong dimension of point)
                    return -1;
                }
            }
            else{
                // NULL input error (missing row in matrix)
                return -1;
            }
        }
        if(a[d] == NULL){
            // NULL input error (missing row in matrix)
            return -1;
        }
        if(tempArray != NULL){
            for(int i = 1; i < size ; i = i*2){
                for(int i2 = 0; i2 < size ; i2 = i2 + i*2){
                    i3 = i2;
                    i4 = i2+i;
                    for(i5 = i2;(i3 < size && i4 < size) && (i3 < i2+i && i4<i2 + i*2) ; i5++){
                        if(spPointGetAxisCoor(data[a[d][i4]],d) < spPointGetAxisCoor(data[a[d][i3]],d)){
                            tempArray[i5] = a[d][i4]; /* Point a[d][i4] comes before point a[d][i3] in dimension d */
                            i4 = i4+1;
                        }
                        else{
                            tempArray[i5] = a[d][i3]; /* Point a[d][i3] comes before point a[d][i4] in dimension d */
                            i3 = i3+1;
                        }
                    }
                    for(; i3 < size && i3 < i2+i ; i5++){
                        tempArray[i5] = a[d][i3]; /* The rest of the points in the first group */
                        i3 = i3+1;
                    }
                    for(; i4 < size && i4 < i2 + i*2 ; i5++){
                        tempArray[i5] = a[d][i4]; /* The rest of the points in the second group */
                        i4 = i4+1;
                    }
                }
                for(int i2 = 0; i2 < size ; i2++)
                    a[d][i2] = tempArray[i2]; /* Moving the result of the merge back to row d */
            }
            return 1;
        }
    }
    return -1;
}

/**
 * Returns the dimension of the points in the KD array.
 *
 * @return
 * The output is dim.
 */
int spKDArrayGetDimension(SPKDArray* kdA){
    if(kdA == NULL)
        return 0;
    return kdA->dim;
}

/**
 * Returns the number of points in the KD array.
 *
 * @return
 * The output is size.
 */
int spKDArrayGetSize(SPKDArray* kdA){
    if(kdA == NULL)
        return 0;
    return kdA->size;
}

/**
 * Returns the array of points in the KD array.
 *
 * @return
 * The output is arr.
 */
SPPoint** spKDArrayGetArray(SPKDArray* kdA){
    if(kdA == NULL)
        return NULL;
    return kdA->arr;
}

/**
 * Returns the indeices of the array of points, a sorted by their dim coordinate.
 *
 * @return
 * The output is arrIndices[dim-1].
 */
int* spKDArrayGetIndicesByDim(SPKDArray* kdA, int dim){
    if(kdA == NULL || dim<1)
        return NULL;
    if(kdA->arrIndices == NULL)
        return NULL;
    return kdA->arrIndices[dim-1];
}

/**
 * Frees all allocated memory of kdA.
 *
 */
void spKDArrayDestroy(SPKDArray* kdA){
    if(kdA != NULL){
        for(int i = 0; i< kdA->dim; i++)
            free(kdA->arrIndices[i]);
        free(kdA->arr);
        free(kdA->arrIndices);
        free(kdA);
    }
}
