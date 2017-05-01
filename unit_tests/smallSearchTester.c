#include <stdio.h>
#include <stdlib.h>
#include "../SPPoint.h"
#include "../SPKDArray.h"
#include "../SPBPriorityQueue.h"
#include "../SPKDTree.h"
#include "../SPLogger.h"

int main()
{
	spLoggerCreate(NULL, SP_LOGGER_ERROR_LEVEL);
    int numOfImages = 4;
    int* numOfFeatures = (int*) malloc(numOfImages*(sizeof(int)));
    numOfFeatures[0] = 3;
    numOfFeatures[1] = 2;
    numOfFeatures[2] = 3;
    numOfFeatures[3] = 3;
    SPPoint*** m =  (SPPoint***) malloc(numOfImages*(sizeof(*m)));
    m[0] =  (SPPoint**) malloc(numOfFeatures[0]*(sizeof(SPPoint*)));
    m[1] =  (SPPoint**) malloc(numOfFeatures[1]*(sizeof(SPPoint*)));
    m[2] =  (SPPoint**) malloc(numOfFeatures[2]*(sizeof(SPPoint*)));
    m[3] =  (SPPoint**) malloc(numOfFeatures[3]*(sizeof(SPPoint*)));
    int dim = 3;
    double* coor = (double*) malloc(dim*(sizeof(double)));

    int index = 0;
    coor[0] = 0; coor[1] = 0;  coor[2] = 0;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 1; coor[1] = 1;  coor[2] = 1;
    m[index][1] = spPointCreate(coor, dim, index);
    coor[0] = 0; coor[1] = 0.5;  coor[2] = 1;
    m[index][2] = spPointCreate(coor, dim, index);

    index = 1;
    coor[0] = 0.1; coor[1] = 0;  coor[2] = 0;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 10; coor[1] = 9;  coor[2] = 7;
    m[index][1] = spPointCreate(coor, dim, index);

    index = 2;
    coor[0] = 0.5; coor[1] = -0.5;  coor[2] = 0;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 1.5; coor[1] = 0.6;  coor[2] = 1;
    m[index][1] = spPointCreate(coor, dim, index);
    coor[0] = -10; coor[1] = -9;  coor[2] = -7;
    m[index][2] = spPointCreate(coor, dim, index);

    index = 3;
    coor[0] = 10; coor[1] = 9;  coor[2] = 7;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 8; coor[1] = 9;  coor[2] = 7;
    m[index][1] = spPointCreate(coor, dim, index);
    coor[0] = 10; coor[1] = 0;  coor[2] = 0;
    m[index][2] = spPointCreate(coor, dim, index);

    SPKDTreeNode* tIncremental = fullKDTreeCreator(m , numOfImages, numOfFeatures, INCREMENTAL);
    SPKDTreeNode* tRandom = fullKDTreeCreator(m , numOfImages, numOfFeatures, RANDOM);
    SPKDTreeNode* tSpread = fullKDTreeCreator(m , numOfImages, numOfFeatures, MAX_SPREAD);

    for(int i = 0; i<numOfImages; i++){
        printf("\nImage %d:\n", i);
        for(int j = 0; j< numOfFeatures[i]; j++){
            for(int k = 0; k< dim; k++)
                    printf("%f , ", spPointGetAxisCoor(m[i][j],k));
            printf("\n");
        }
    }
    int kNN = 2;
    int targetImage = 1;
    int numOfClosest = numOfImages;
    int* closest1 = (int*) malloc(sizeof(int) * numOfClosest);
    closestImagesSearch(kNN, closest1, numOfClosest, m[targetImage], numOfFeatures[targetImage], tIncremental, numOfImages);
    printf("\n\n%d closest neighbours to point %d test (kNN: %d): \n\n", numOfClosest, targetImage, kNN);
    for(int i = 0; i<numOfClosest; i++)
        printf("%d , ", closest1[i]);

    free (closest1);
    spKDTreeDestroy(tIncremental);
    spKDTreeDestroy(tRandom);
    spKDTreeDestroy(tSpread);
    spLoggerDestroy();
    return 0;
}

