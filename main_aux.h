#ifndef MAIN_AUX_H_
#define MAIN_AUX_H_

extern "C" {
#include "SPLogger.h"
#include "SPConfig.h"
#include "SPPoint.h"
}



SPConfig spInit(int argc, char* argv[]);

SPPoint*** spPreprocessing(int** NOFptr, const SPConfig config);

void destroySPPoint2D(SPPoint ***DB, int dim, int *dim2);


#endif /* MAIN_AUX_H_ */
