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
	SPPoint** arr;
	int** arrIndices;
	int dim;
	int size;
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
    int i = 0;
    int j = 0;
    int d = 0;
    if(size>0 && arr != NULL){
        for(i = 0; i<size ; i++){
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
            int **a = (int**) malloc(d*sizeof(*a));
            if(a == NULL)
                return NULL;
            int* tempArray = (int*) malloc(size*sizeof(int));
            if(tempArray == NULL){
                free(a);
                return NULL;
            }
            for(i = 0; i<d ; i++){
                a[i] = (int*) malloc(size*sizeof(int));
                if(a[i] == NULL){
                    d = -1;
                }
                else{
                    for(j = 0; j<size; j++)
                        a[i][j] = j;
                    spSortPointArrayByDimension(a , arr , size, i, tempArray);
                }
            }
            free(tempArray);
            if(d == -1){
                for(i--; i>-1 ; i--){
                    free(a[i]);
                }
                free(a);
                return NULL;
            }
            SPPoint** dataCopy = spCopyPointArray(arr , size);
            if(dataCopy == NULL){
                for(i--; i>-1 ; i--){
                    free(a[i]);
                }
                free(a);
                return NULL;
            }
            SPKDArray* res = spKDArrayInitPreSorted(dataCopy, a, size , d);
            if(res == NULL)
            {
                for(i = d-1; i>-1 ; i--)
                    free(a[i]);
                free(a);
                free(dataCopy);
                return NULL;
            }
            return res;
        }
    }
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
    return NULL;
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
    if(kdArr == NULL)
        return NULL;
    if((coor < 1 || coor > kdArr->dim) || kdArr->size < 2)
        return NULL;
    int n1 = kdArr->size;
    if(n1 % 2 == 1)
        n1 = n1+1;
    n1 = (int) n1/2;
    int n2 = kdArr->size - n1;
    int i = 0;
    int k = 0;
    int j1 = 0;
    int j2 = 0;
    SPKDArray** res = (SPKDArray**) malloc(2*(sizeof(kdArr)));
    int* tempSplitArray = (int*) malloc((kdArr->size)*sizeof(int));
    int* tempNewIndex = (int*) malloc((kdArr->size)*sizeof(int));
    SPPoint **dataLeft = (SPPoint**) malloc(n1*sizeof(*dataLeft));
    SPPoint **dataRight = (SPPoint**) malloc(n2*sizeof(*dataRight));
    int **aLeft = (int**) malloc((kdArr->dim)*sizeof(*aLeft));
    int **aRight = (int**) malloc((kdArr->dim)*sizeof(*aRight));
    if(aRight != NULL && aLeft != NULL){
        for(k = 0; k< kdArr->dim ; k++){
            aLeft[k] = (int*) malloc((n1)*sizeof(int));
            if(aLeft[k] == NULL)
                k = k+2*kdArr->dim;
            else{
                aRight[k] = (int*) malloc((n2)*sizeof(int));
                if(aRight[k] == NULL){
                    free(aLeft[k]);
                    k = k+2*kdArr->dim;
                }
            }
        }
        if(k > 2*kdArr->dim-1){
            for(k = k - (2*kdArr->dim+1);k>-1;k--){
                free(aLeft[k]);
                free(aRight[k]);
            }
        }
    }
    if((((dataRight == NULL || dataLeft == NULL) || (res == NULL || tempSplitArray == NULL)) || (aRight == NULL || aLeft == NULL)) || (tempNewIndex == NULL || k<0)){
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
    for(i = 0; i< kdArr->size ; i++){
        tempSplitArray[kdArr->arrIndices[coor-1][i]] = i;
    }
    for(i = 0; i< kdArr->size ; i++){
        if(tempSplitArray[i]<n1){
            dataLeft[j1] = kdArr->arr[i];
            tempNewIndex[i] = j1;
            j1++;
        }
        else{
            dataRight[j2] = kdArr->arr[i];
            tempNewIndex[i] = j2;
            j2++;
        }
    }
    for(k = 0; k< kdArr->dim ; k++){
        j1 = 0;
        j2 = 0;
        for(i = 0; i< kdArr->size ; i++){
            if(tempSplitArray[kdArr->arrIndices[k][i]]<n1){
                aLeft[k][j1] = tempNewIndex[kdArr->arrIndices[k][i]];
                j1++;
            }
            else{
                aRight[k][j2] = tempNewIndex[kdArr->arrIndices[k][i]];
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
 * Makes a new copy of the pointers of an SPPoint array.
 *
 * @return
 * The copy is returned (NULL in case of allocation error)
 */
SPPoint** spCopyPointArray(SPPoint** base, int size ){
    int i=0;
    int j=0;
    if(size>0 && base != NULL){
        SPPoint **res = (SPPoint**) malloc(size*(sizeof(*res)));
        if(res != NULL){
            for(i = 0; i < size; i++){
                res[i] = NULL;
                if(base[i] != NULL)
                    res[i] = base[i];
                if(res[i] == NULL){
                    i = size+1;
                }
            }
            if(i != size){
                free(res);
                return NULL;
            }
            return res;
        }
    }
    return NULL;
}

/**
 * Sorts row number d of the inputed matrix a by the coordinates of the points in the inputed dimension.
 * The sort is logarithmic, complexity O(size log(size)). It requires an int array with size elements
 * for temporary sorting purposes.
 *
 * @return
 * 1 if sort succeeded (-1 in case of error)
 */
int spSortPointArrayByDimension(int** a , SPPoint** data, int size, int d , int* tempArray){
    int i = 0;
    int i2 = 0;
    int i3 = 0;
    int i4 = 0;
    int i5 = 0;
    int dim = 0;
    if((size>0 && d>-1) && (a != NULL && data != NULL)){
        for(i = 0; i < size; i++){
            if(data[i] != NULL){
                if(i == 0)
                    dim = spPointGetDimension((data[i]));
                if(spPointGetDimension((data[i])) != dim){
                    return -1;
                }
            }
            else
                return -1;
        }
        if(a[d] == NULL)
            return -1;
        if(tempArray != NULL){
            i = 2;
            for(i = 1; i < size ; i = i*2){
                for(i2 = 0; i2 < size ; i2 = i2 + i*2){
                    i3 = i2;
                    i4 = i2+i;
                    for(i5 = i2;(i3 < size && i4 < size) && (i3 < i2+i && i4<i2 + i*2) ; i5++){
                        if(spPointGetAxisCoor(data[a[d][i4]],d) < spPointGetAxisCoor(data[a[d][i3]],d)){
                            tempArray[i5] = a[d][i4];
                            i4 = i4+1;
                        }
                        else{
                            tempArray[i5] = a[d][i3];
                            i3 = i3+1;
                        }
                    }
                    for(; i3 < size && i3 < i2+i ; i5++){
                        tempArray[i5] = a[d][i3];
                        i3 = i3+1;
                    }
                    for(; i4 < size && i4 < i2 + i*2 ; i5++){
                        tempArray[i5] = a[d][i4];
                        i4 = i4+1;
                    }
                }
                for(i2 = 0; i2 < size ; i2++)
                    a[d][i2] = tempArray[i2];
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

