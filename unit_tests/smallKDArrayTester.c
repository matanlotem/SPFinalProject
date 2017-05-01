#include <stdio.h>
#include <stdlib.h>
#include "../SPPoint.h"
#include "../SPKDArray.h"
#include "../SPLogger.h"


void destroyPointArray(SPPoint** dataCopy , int size1){
    if(dataCopy != NULL){
        for(int i = 0; i < size1; i++)
            spPointDestroy(dataCopy[i]);
        free(dataCopy);
    }
}
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
	spLoggerCreate(NULL, SP_LOGGER_ERROR_LEVEL);
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
    printf("kdA: ");
    printKDArray(kdA);
    SPKDArray** kdA1 = spKDArraySplit(kdA ,1);
    printf("\n\nkdA Split by first coordinate. kd1Left:\n");
    printKDArray(kdA1[0]);
    printf("\nkd1Right:\n");
    printKDArray(kdA1[1]);
    SPKDArray** kdA2 = spKDArraySplit(kdA ,2);
    printf("\n\nkdA Split by second coordinate. kd2Left:\n");
    printKDArray(kdA2[0]);
    printf("\nkd2Right:\n");
    printKDArray(kdA2[1]);
    SPKDArray** kdA3 = spKDArraySplit(kdA ,3);
    printf("\n\nkdA Split by third coordinate. kd3Left:\n");
    printKDArray(kdA3[0]);
    printf("\nkd3Right:\n");
    printKDArray(kdA3[1]);

    SPKDArray** kdA32 = spKDArraySplit(kdA3[1] ,2);
    printf("\n\nkd3Right Split by first coordinate. kd32Left:\n");
    printKDArray(kdA32[0]);
    printf("\nkd32Right:\n");
    printKDArray(kdA32[1]);

    SPKDArray** kdA321 = spKDArraySplit(kdA32[0] ,2);
    printf("\n\nkd32Left Split by first coordinate. kd321Left:\n");
    printKDArray(kdA321[0]);
    printf("\nkd321Right:\n");
    printKDArray(kdA321[1]);

    spKDArrayDestroy(kdA);
    spKDArrayDestroy(kdA321[0]);
    spKDArrayDestroy(kdA321[1]);
    free(kdA321);
    spKDArrayDestroy(kdA32[0]);
    spKDArrayDestroy(kdA32[1]);
    free(kdA32);
    spKDArrayDestroy(kdA3[0]);
    spKDArrayDestroy(kdA3[1]);
    free(kdA3);
    spKDArrayDestroy(kdA2[0]);
    spKDArrayDestroy(kdA2[1]);
    free(kdA2);
    spKDArrayDestroy(kdA1[0]);
    spKDArrayDestroy(kdA1[1]);
    free(kdA1);
    spLoggerDestroy();
    free(coor);
    destroyPointArray(dataCopy , size1);
    return 0;
}


