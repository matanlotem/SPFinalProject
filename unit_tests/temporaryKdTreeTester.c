#include <stdio.h>
#include <stdlib.h>
#include "SPPoint.h"
#include "SPKDArray.h"
#include "SPBPriorityQueue.h"
#include "SPKDTree.h"
#include "SPConfig.h"

/*
** This function uses a less efficient method to calculate the kNN closest features to
** each target feature m[searchIndex][i], and return the image indices in order of closeness.
 */
int* findClosestByHand(SPPoint*** m, int numOfImages, int* numOfFeatures, int searchIndex, int kNN){
    int* res = (int*) malloc(numOfImages*(sizeof(int)));
    int* prevRes = (int*) malloc(numOfImages*(sizeof(int)));
    int* counterF = (int*) malloc(numOfImages*(sizeof(int)));
    int numOfFeat = 0;
    int k = 0;
    int k2 = 0;
    for(int i = 0; i < numOfImages; i++){
        res[i] = i;
        counterF[i] = 0;
        numOfFeat = numOfFeat+numOfFeatures[i];
    }
    int* allPointsI = (int*) malloc(numOfFeat*sizeof(int));
    int* allPointsImageIndex = (int*) malloc(numOfFeat*sizeof(int));
    SPPoint** allPoints = (SPPoint**) malloc(numOfFeat*sizeof(*allPoints));
    for(int i = 0; i < numOfImages; i++){
        for(int j = 0; j < numOfFeatures[i]; j++){
            allPoints[k] = m[i][j];
            allPointsImageIndex[k] = i;
            k = k+1;
        }
    }
    for(int i = 0; i < numOfFeatures[searchIndex]; i++){
        for(int j = 0; j<numOfImages ; j++){
            prevRes[j] = 0;
        }
        for(int j = 0; j<numOfFeat ; j++){
            allPointsI[j] = j;
        }
        k = 1;
        while(k < numOfFeat){
            if(k < 1)
                k = 1;
            else{
                if(spPointL2SquaredDistance(m[searchIndex][i], allPoints[allPointsI[k]]) < spPointL2SquaredDistance(m[searchIndex][i], allPoints[allPointsI[k-1]])){
                    k2 = allPointsI[k];
                    allPointsI[k] = allPointsI[k-1];
                    allPointsI[k-1] = k2;
                    k = k-1;
                }
                else
                    k = k+1;
            }
        }
        for(int j = 0; j < kNN; j++){
            if(prevRes[allPointsImageIndex[allPointsI[j]]] == 0){
                prevRes[allPointsImageIndex[allPointsI[j]]] = 0;
                counterF[allPointsImageIndex[allPointsI[j]]] = counterF[allPointsImageIndex[allPointsI[j]]] + 1;
            }
        }
    }
    k = 1;
    while(k < numOfImages){
        if(k < 1)
            k = 1;
        else{
            if(counterF[res[k]] > counterF[res[k-1]]){
                k2 = res[k];
                res[k] = res[k-1];
                res[k-1] = k2;
                k = k-1;
            }
            else
                k = k+1;
        }
    }
//    printf("\n Real closeness:\n");
//    for(int j = 0; j < numOfImages; j++){
//        printf("%d: %d, ",j,counterF[j]);
//    }
//    printf("\n");

    free(allPoints);
    free(prevRes);
    free(counterF);
    return res;
}

/*
** This function perform the closest images search using a set of features of images defined in it,
** that are mostly identical and otherwise unhelpful.
 */
void treeTesterPoor(int kNN, int numOfClosest, KD_METHOD splitMethod, int searchIndex,int numOfImagesTest, int* numOfFeaturesTest)
{
    int numOfImages = 5; // Image definitions
    int* numOfFeatures = (int*) malloc(numOfImages*(sizeof(int)));
    numOfFeatures[0] = 7;
    numOfFeatures[1] = 7;
    numOfFeatures[2] = 7;
    numOfFeatures[3] = 7;
    numOfFeatures[4] = 7;
    SPPoint*** m =  (SPPoint***) malloc(numOfImages*(sizeof(*m)));
    m[0] =  (SPPoint**) malloc(numOfFeatures[0]*(sizeof(SPPoint*)));
    m[1] =  (SPPoint**) malloc(numOfFeatures[1]*(sizeof(SPPoint*)));
    m[2] =  (SPPoint**) malloc(numOfFeatures[2]*(sizeof(SPPoint*)));
    m[3] =  (SPPoint**) malloc(numOfFeatures[3]*(sizeof(SPPoint*)));
    m[4] =  (SPPoint**) malloc(numOfFeatures[4]*(sizeof(SPPoint*)));
    int dim = 3;
    double* coor = (double*) malloc(dim*(sizeof(double)));

    int index = 0;
    coor[0] = 00; coor[1] = 00;  coor[2] = 00;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 11; coor[1] = 11;  coor[2] = 11;
    m[index][1] = spPointCreate(coor, dim, index);
    coor[0] = 20; coor[1] = 20.5;  coor[2] = 21;
    m[index][2] = spPointCreate(coor, dim, index);
    coor[0] = 31; coor[1] = 32;  coor[2] = 33;
    m[index][3] = spPointCreate(coor, dim, index);
    coor[0] = 41; coor[1] = 41;  coor[2] = 41;
    m[index][4] = spPointCreate(coor, dim, index);
    coor[0] = 52; coor[1] = 52;  coor[2] = 50.3;
    m[index][5] = spPointCreate(coor, dim, index);
    coor[0] = 60; coor[1] = 60;  coor[2] = 60.5;
    m[index][6] = spPointCreate(coor, dim, index);

    index = 1;
    coor[0] = 00; coor[1] = 00;  coor[2] = 00;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 11; coor[1] = 11;  coor[2] = 11;
    m[index][1] = spPointCreate(coor, dim, index);
    coor[0] = 20; coor[1] = 20.5;  coor[2] = 21;
    m[index][2] = spPointCreate(coor, dim, index);
    coor[0] = 31; coor[1] = 32;  coor[2] = 33;
    m[index][3] = spPointCreate(coor, dim, index);
    coor[0] = 41; coor[1] = 41;  coor[2] = 41;
    m[index][4] = spPointCreate(coor, dim, index);
    coor[0] = 52; coor[1] = 52;  coor[2] = 50.3;
    m[index][5] = spPointCreate(coor, dim, index);
    coor[0] = 60; coor[1] = 60;  coor[2] = 60.5;
    m[index][6] = spPointCreate(coor, dim, index);

    index = 2;
    coor[0] = 00; coor[1] = 00;  coor[2] = 03;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 11; coor[1] = 11;  coor[2] = 13;
    m[index][1] = spPointCreate(coor, dim, index);
    coor[0] = 20; coor[1] = 20.5;  coor[2] = 23;
    m[index][2] = spPointCreate(coor, dim, index);
    coor[0] = 31; coor[1] = 32;  coor[2] = 34;
    m[index][3] = spPointCreate(coor, dim, index);
    coor[0] = 41; coor[1] = 41;  coor[2] = 43;
    m[index][4] = spPointCreate(coor, dim, index);
    coor[0] = 52; coor[1] = 52;  coor[2] = 53.3;
    m[index][5] = spPointCreate(coor, dim, index);
    coor[0] = 62; coor[1] = 60;  coor[2] = 60.5;
    m[index][6] = spPointCreate(coor, dim, index);

    index = 3;
    coor[0] = 00; coor[1] = 02.4;  coor[2] = 00;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 11; coor[1] = 14.4;  coor[2] = 11;
    m[index][1] = spPointCreate(coor, dim, index);
    coor[0] = 20; coor[1] = 20.4;  coor[2] = 21;
    m[index][2] = spPointCreate(coor, dim, index);
    coor[0] = 31; coor[1] = 34.4;  coor[2] = 33;
    m[index][3] = spPointCreate(coor, dim, index);
    coor[0] = 41; coor[1] = 43.4;  coor[2] = 41;
    m[index][4] = spPointCreate(coor, dim, index);
    coor[0] = 52; coor[1] = 52.4;  coor[2] = 50.3;
    m[index][5] = spPointCreate(coor, dim, index);
    coor[0] = 60; coor[1] = 61.4;  coor[2] = 60.5;
    m[index][6] = spPointCreate(coor, dim, index);

    index = 4;
    coor[0] = 00; coor[1] = 00;  coor[2] = 00;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 11; coor[1] = 11;  coor[2] = 11;
    m[index][1] = spPointCreate(coor, dim, index);
    coor[0] = 20; coor[1] = 20.5;  coor[2] = 21;
    m[index][2] = spPointCreate(coor, dim, index);
    coor[0] = 31; coor[1] = 32;  coor[2] = 33;
    m[index][3] = spPointCreate(coor, dim, index);
    coor[0] = 41; coor[1] = 41;  coor[2] = 41;
    m[index][4] = spPointCreate(coor, dim, index);
    coor[0] = 52; coor[1] = 52;  coor[2] = 50.3;
    m[index][5] = spPointCreate(coor, dim, index);
    coor[0] = 60; coor[1] = 60;  coor[2] = 60.5;
    m[index][6] = spPointCreate(coor, dim, index);

     // Tree creation
    SPKDTreeNode* t0 = fullKDTreeCreator(m , numOfImagesTest, numOfFeaturesTest, splitMethod);
    if(t0 != NULL){
//        for(int i = 0; i<numOfImagesTest; i++){
//            printf("\nImage %d:\n", i);
//            for(int j = 0; j< numOfFeaturesTest[i]; j++){
//                for(int k = 0; k< dim; k++)
//                        printf("%f , ", spPointGetAxisCoor(m[i][j],k));
//                printf("\n");
//            }
//        }
         // Tree search
        int* closest1 = (int*) malloc(numOfClosest*(sizeof(double)));
        if(closestImagesSearch(kNN,closest1, numOfClosest, m[searchIndex], numOfFeaturesTest[searchIndex], t0, numOfImagesTest) < 0)
            printf("\n Error\n");
        else{
            int passed = 1;
            int* closestReal =  findClosestByHand(m, numOfImagesTest, numOfFeaturesTest, searchIndex, kNN);
            printf("\n\nPoor set: %d closest neighbours to point %d real (kNN: %d, split method %d): \n\n", numOfClosest, searchIndex, kNN, (int) splitMethod);
            for(int i = 0; i<numOfClosest; i++){
                printf("%d , ", closestReal[i]);
                if(closestReal[i] != closest1[i])
                    passed = 0;
            }
            printf("\n\nPoor set: %d closest neighbours to point %d test (kNN: %d, split method %d): \n\n", numOfClosest, searchIndex, kNN, (int) splitMethod);
            for(int i = 0; i<numOfClosest; i++)
                printf("%d , ", closest1[i]);
            if(passed == 1)
                printf("\n\n Test passed.\n\n");
            else
                printf("\n\n Test not passed (possible if there are different orders of images with same similarity).\n\n");
            free (closestReal);
        }
        free (closest1);
    }
    else{
        printf("\nTree creation error\n");
        for(int i = 0; i < numOfImages; i++){
            for(int j = 0; j < numOfFeatures[i]; j++){
                spPointDestroy(m[i][j]);
            }
        }
    }

    free(coor);
    if(t0 != NULL)
        spKDTreeDestroy(t0);
    for(int i = 0; i < numOfImages; i++){
        free(m[i]);
    }
    free(m);
    free(numOfFeatures);
}

/*
** This function perform the closest images search using a set of features of images defined in it.
 */
void treeTesterGood(int kNN, int numOfClosest, KD_METHOD splitMethod, int searchIndex,int numOfImagesTest, int* numOfFeaturesTest)
{
    int numOfImages = 5; // Image definitions
    int* numOfFeatures = (int*) malloc(numOfImages*(sizeof(int)));
    numOfFeatures[0] = 7;
    numOfFeatures[1] = 7;
    numOfFeatures[2] = 7;
    numOfFeatures[3] = 7;
    numOfFeatures[4] = 7;
    SPPoint*** m =  (SPPoint***) malloc(numOfImages*(sizeof(*m)));
    m[0] =  (SPPoint**) malloc(numOfFeatures[0]*(sizeof(SPPoint*)));
    m[1] =  (SPPoint**) malloc(numOfFeatures[1]*(sizeof(SPPoint*)));
    m[2] =  (SPPoint**) malloc(numOfFeatures[2]*(sizeof(SPPoint*)));
    m[3] =  (SPPoint**) malloc(numOfFeatures[3]*(sizeof(SPPoint*)));
    m[4] =  (SPPoint**) malloc(numOfFeatures[4]*(sizeof(SPPoint*)));
    int dim = 3;
    double* coor = (double*) malloc(dim*(sizeof(double)));

    int index = 0;
    coor[0] = 00; coor[1] = 00;  coor[2] = 00;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 11; coor[1] = 11;  coor[2] = 11;
    m[index][1] = spPointCreate(coor, dim, index);
    coor[0] = 20; coor[1] = 20.5;  coor[2] = 21;
    m[index][2] = spPointCreate(coor, dim, index);
    coor[0] = 31; coor[1] = 32;  coor[2] = 33;
    m[index][3] = spPointCreate(coor, dim, index);
    coor[0] = 41; coor[1] = 41;  coor[2] = 41;
    m[index][4] = spPointCreate(coor, dim, index);
    coor[0] = 52; coor[1] = 52;  coor[2] = 50.3;
    m[index][5] = spPointCreate(coor, dim, index);
    coor[0] = 60; coor[1] = 60;  coor[2] = 60.5;
    m[index][6] = spPointCreate(coor, dim, index);

    index = 1;
    coor[0] = 01; coor[1] = 02;  coor[2] = 03;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 10; coor[1] = 11;  coor[2] = 11;
    m[index][1] = spPointCreate(coor, dim, index);
    coor[0] = 20; coor[1] = 20.5;  coor[2] = 20;
    m[index][2] = spPointCreate(coor, dim, index);
    coor[0] = 33; coor[1] = 32;  coor[2] = 33;
    m[index][3] = spPointCreate(coor, dim, index);
    coor[0] = 40.5; coor[1] = 41;  coor[2] = 41;
    m[index][4] = spPointCreate(coor, dim, index);
    coor[0] = 52; coor[1] = 52;  coor[2] = 50.3;
    m[index][5] = spPointCreate(coor, dim, index);
    coor[0] = 61; coor[1] = 60;  coor[2] = 60.5;
    m[index][6] = spPointCreate(coor, dim, index);

    index = 2;
    coor[0] = 00; coor[1] = 00;  coor[2] = 03;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 11; coor[1] = 11;  coor[2] = 13;
    m[index][1] = spPointCreate(coor, dim, index);
    coor[0] = 20; coor[1] = 20.5;  coor[2] = 23;
    m[index][2] = spPointCreate(coor, dim, index);
    coor[0] = 31; coor[1] = 32;  coor[2] = 34;
    m[index][3] = spPointCreate(coor, dim, index);
    coor[0] = 41; coor[1] = 41;  coor[2] = 43;
    m[index][4] = spPointCreate(coor, dim, index);
    coor[0] = 52; coor[1] = 52;  coor[2] = 53.3;
    m[index][5] = spPointCreate(coor, dim, index);
    coor[0] = 62; coor[1] = 60;  coor[2] = 60.5;
    m[index][6] = spPointCreate(coor, dim, index);

    index = 3;
    coor[0] = 00; coor[1] = 02.4;  coor[2] = 00;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 11; coor[1] = 14.4;  coor[2] = 11;
    m[index][1] = spPointCreate(coor, dim, index);
    coor[0] = 20; coor[1] = 20.4;  coor[2] = 21;
    m[index][2] = spPointCreate(coor, dim, index);
    coor[0] = 31; coor[1] = 34.4;  coor[2] = 33;
    m[index][3] = spPointCreate(coor, dim, index);
    coor[0] = 41; coor[1] = 43.4;  coor[2] = 41;
    m[index][4] = spPointCreate(coor, dim, index);
    coor[0] = 52; coor[1] = 52.4;  coor[2] = 50.3;
    m[index][5] = spPointCreate(coor, dim, index);
    coor[0] = 60; coor[1] = 61.4;  coor[2] = 60.5;
    m[index][6] = spPointCreate(coor, dim, index);

    index = 4;
    coor[0] = 00; coor[1] = 00;  coor[2] = 00;
    m[index][0] = spPointCreate(coor, dim, index);
    coor[0] = 11; coor[1] = 11;  coor[2] = 11;
    m[index][1] = spPointCreate(coor, dim, index);
    coor[0] = 20; coor[1] = 20.5;  coor[2] = 21;
    m[index][2] = spPointCreate(coor, dim, index);
    coor[0] = 31; coor[1] = 32;  coor[2] = 33;
    m[index][3] = spPointCreate(coor, dim, index);
    coor[0] = 41; coor[1] = 41;  coor[2] = 41;
    m[index][4] = spPointCreate(coor, dim, index);
    coor[0] = 52; coor[1] = 52;  coor[2] = 50.3;
    m[index][5] = spPointCreate(coor, dim, index);
    coor[0] = 60; coor[1] = 60;  coor[2] = 60.5;
    m[index][6] = spPointCreate(coor, dim, index);

    // Tree creation
    SPKDTreeNode* t0 = fullKDTreeCreator(m , numOfImagesTest, numOfFeaturesTest, splitMethod);
    if(t0 != NULL){
//        for(int i = 0; i<numOfImagesTest; i++){
//            printf("\nImage %d:\n", i);
//            for(int j = 0; j< numOfFeaturesTest[i]; j++){
//                for(int k = 0; k< dim; k++)
//                        printf("%f , ", spPointGetAxisCoor(m[i][j],k));
//                printf("\n");
//            }
//        }
        // tree search
        int* closest1 = (int*) malloc(numOfClosest*(sizeof(double)));
        if(closestImagesSearch(kNN,closest1, numOfClosest, m[searchIndex], numOfFeaturesTest[searchIndex], t0, numOfImagesTest) < 0)
            printf("\n Error\n");
        else{
            int passed = 1;
            int* closestReal =  findClosestByHand(m, numOfImagesTest, numOfFeaturesTest, searchIndex, kNN);
            printf("\n\n%d closest neighbours to point %d real (kNN: %d, split method %d): \n\n", numOfClosest, searchIndex, kNN, (int) splitMethod);
            for(int i = 0; i<numOfClosest; i++){
                printf("%d , ", closestReal[i]);
                if(closestReal[i] != closest1[i])
                    passed = 0;
            }
            printf("\n\n%d closest neighbours to point %d test (kNN: %d, split method %d): \n\n", numOfClosest, searchIndex, kNN, (int) splitMethod);
            for(int i = 0; i<numOfClosest; i++)
                printf("%d , ", closest1[i]);
            if(passed == 1)
                printf("\n\n Test passed.\n\n");
            else
                printf("\n\n Test not passed (possible if there are different orders of images with same similarity).\n\n");
            free (closestReal);
        }
        free (closest1);
    }
    else{
        printf("\nTree creation error\n");
        for(int i = 0; i < numOfImages; i++){
            for(int j = 0; j < numOfFeatures[i]; j++){
                spPointDestroy(m[i][j]);
            }
        }
    }

    free(coor);
    if(t0 != NULL)
        spKDTreeDestroy(t0);
    for(int i = 0; i < numOfImages; i++){
        free(m[i]);
    }
    free(m);
    free(numOfFeatures);
}

/*
** This function contains the maximum feature numbers in this tester, and returns maximum image number.
*/
int resetFeatureNumbers(int* numOfFeatures){
    if(numOfFeatures != NULL){
        numOfFeatures[0] = 7;
        numOfFeatures[1] = 7;
        numOfFeatures[2] = 7;
        numOfFeatures[3] = 7;
        numOfFeatures[4] = 7;
    }
    return 5;
}

int main()
{
    // first, the maximum sizes for images are defined.
    int* numOfFeatures = (int*) malloc(5*(sizeof(int)));
    int numOfImages = resetFeatureNumbers(numOfFeatures);
    int kNN = 3;
    int numOfClosest = 5;
    int searchIndex = 0;
    KD_METHOD splitMethod = RANDOM;

    printf("\nTest 1:");
    kNN = 1;
    searchIndex = 0;
    splitMethod = RANDOM;
    treeTesterGood(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    treeTesterPoor(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    numOfImages = resetFeatureNumbers(numOfFeatures);

    printf("\nTest 2:");
    kNN = 2;
    searchIndex = 1;
    splitMethod = INCREMENTAL;
    treeTesterGood(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    treeTesterPoor(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    numOfImages = resetFeatureNumbers(numOfFeatures);

    printf("\nTest 3:");
    kNN = 3;
    searchIndex = 2;
    splitMethod = MAX_SPREAD;
    treeTesterGood(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    treeTesterPoor(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    numOfImages = resetFeatureNumbers(numOfFeatures);

    printf("\nTest 4:");
    kNN = 4;
    searchIndex = 3;
    splitMethod = MAX_SPREAD;
    treeTesterGood(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    treeTesterPoor(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    numOfImages = resetFeatureNumbers(numOfFeatures);

    printf("\nTest 5:");
    kNN = 5;
    searchIndex = 4;
    splitMethod = MAX_SPREAD;
    treeTesterGood(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    treeTesterPoor(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    numOfImages = resetFeatureNumbers(numOfFeatures);

    printf("\nTest 6 (point 3 has less features):");
    kNN = 2;
    numOfFeatures[3] = 4;
    searchIndex = 2;
    splitMethod = MAX_SPREAD;
    treeTesterGood(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    treeTesterPoor(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    numOfImages = resetFeatureNumbers(numOfFeatures);

    printf("\nTest 7 (point 3 has less features):");
    kNN = 2;
    numOfFeatures[3] = 4;
    searchIndex = 3;
    splitMethod = MAX_SPREAD;
    treeTesterGood(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    treeTesterPoor(kNN,  numOfClosest,  splitMethod,  searchIndex, numOfImages, numOfFeatures);
    numOfImages = resetFeatureNumbers(numOfFeatures);


    free(numOfFeatures);
    return 0;
}

