
#include "Test.h"

#define TEST_LIST 0
#define TEST_FUNC 0
#define TEST_SPEED 0
#define TEST_MEM 1

using namespace std;

int main() {
#if TEST_LIST
    testList();
#endif
#if TEST_FUNC
    testBPTreeFunction();
#endif
#if TEST_SPEED
    testBPTreeSpeed();
#endif
#if TEST_MEM
    testBPTreeMemory();
#endif
    return 0;
}
