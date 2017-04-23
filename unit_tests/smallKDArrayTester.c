#include <stdio.h>
#include <stdlib.h>
#include "../SPPoint.h"
#include "../SPKDArray.h"


void printKDArray(SPKDArray* kdA){
    if(kdA != NULL){
        int i = 0;
        int j = 0;
        int n = spKDArrayGetSize(kdA);
        int d = spKDArrayGetDimension(kdA);
        SPPoint** p = spKDArrayGetArray(kdA);
        printf("There are %d points with %d dimensions:", n , d);
        for(i = 0; i < n ; i++){
            printf("\nPoint %d: ",i);
            for(j = 0; j<d;j++){
                printf("%f , ", spPointGetAxisCoor(p[i], j));
            }
        }
        printf("\n\nIndices by dimensions:");
        for(j = 0; j<d;j++){
            printf("\nDimension %d: ",j);
            for(i = 0; i < n ; i++)
                printf("%d , ", (spKDArrayGetIndicesByDim(kdA, j+1))[i]);
        }
    }
}
int main()
{
    int size1 = 7;
    int dim = 3;
    int index = 1;
    double* coor = (double*) malloc(dim*(sizeof(double)));
    SPPoint** dataCopy = (SPPoint**) malloc(size1*(sizeof(*dataCopy)));
    coor[0] = 1; coor[1] = 2;  coor[2] = 3;
    dataCopy[0] = spPointCreate(coor, dim, index); index++;
    coor[0] = 1; coor[1] = 3;  coor[2] = 2;
    dataCopy[1] = spPointCreate(coor, dim, index); index++;
    coor[0] = 2; coor[1] = 1;  coor[2] = 3;
    dataCopy[2] = spPointCreate(coor, dim, index); index++;
    coor[0] = 2; coor[1] = 3;  coor[2] = 1;
    dataCopy[3] = spPointCreate(coor, dim, index); index++;
    coor[0] = 3; coor[1] = 2;  coor[2] = 1;
    dataCopy[4] = spPointCreate(coor, dim, index); index++;
    coor[0] = 3; coor[1] = 1;  coor[2] = 2;
    dataCopy[5] = spPointCreate(coor, dim, index); index++;
    coor[0] = 4; coor[1] = 4;  coor[2] = 0;
    dataCopy[6] = spPointCreate(coor, dim, index); index++;

    SPKDArray* kdA = spKDArrayInit(dataCopy, size1);
    printKDArray(kdA);
    SPKDArray** kdA2 = spKDArraySplit(kdA ,1);
    printf("\n\nSplit by first coordinate. kdLeft:\n");
    printKDArray(kdA2[0]);
    printf("\nkdRight:\n");
    printKDArray(kdA2[1]);
    kdA2 = spKDArraySplit(kdA ,2);
    printf("\n\nSplit by second coordinate. kdLeft:\n");
    printKDArray(kdA2[0]);
    printf("\nkdRight:\n");
    printKDArray(kdA2[1]);
    kdA2 = spKDArraySplit(kdA ,3);
    printf("\n\nSplit by third coordinate. kdLeft:\n");
    printKDArray(kdA2[0]);
    printf("\nkdRight:\n");
    printKDArray(kdA2[1]);
    printf("Hello world!\n");

    SPKDArray** kdA3 = spKDArraySplit(kdA2[1] ,2);
    printf("\n\nSplit by first coordinate. kdLeft:\n");
    printKDArray(kdA3[0]);
    printf("\nkdRight:\n");
    printKDArray(kdA3[1]);

    spKDArrayDestroy(kdA);
    spKDArrayDestroy(kdA3[0]);
    spKDArrayDestroy(kdA3[1]);
    spKDArrayDestroy(kdA2[0]);
    spKDArrayDestroy(kdA2[1]);
    free(kdA2);
    free(kdA3);
    return 0;
}

