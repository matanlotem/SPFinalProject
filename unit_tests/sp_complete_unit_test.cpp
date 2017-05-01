#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdbool>
#include "../main_aux.h"

extern "C" {
#include "unit_test_util.h" //SUPPORTING MACROS ASSERT_TRUE/ASSERT_FALSE etc..
#include "../SPConsts.h"
}

#define TEST_DIR "unit_tests/sp_complete/"

bool selfImageTest() {
	return true;

}

int main(int argc, char* argv[]) {
	if (argc > 1) printf("%s\n",argv[0]);
	RUN_TEST(selfImageTest);
	return 0;
}
