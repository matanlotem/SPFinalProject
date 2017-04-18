#include <cstdlib>
#include <cstdio>
#include <cstring>
extern "C" {
#include "main_aux.h"
}

int main(int argc, char* argv[]) {
	if (argc < 1)
		return -1;
	else
		printf("%s\n",argv[0]);

	SP_CONFIG_MSG configMsg;
	SPConfig config = spInit(argc, argv);
	if (!config) return -1;

	int* NOFptr;
	SPPoint*** featsDB = spPreprocessing(&NOFptr, config);
	if (!featsDB) {
		spConfigDestroy(config);
		return -1;
	}

	destroySPPoint2D(featsDB,spConfigGetNumOfImages(config, &configMsg), NOFptr);
	spConfigDestroy(config);

	printf("yay\n");
	return 0;

}
