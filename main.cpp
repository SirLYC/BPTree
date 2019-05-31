
#include "Test.h"
#include "BPTree.h"
#include <iostream>
#include "Utils.h"

#define TEST_LIST 0
#define TEST_FUNC 0
#define TEST_SPEED 1
#define TEST_MEM 0
#define TEST_SERIAL 0

using namespace std;

int main() {
#if TEST_LIST
    testList();
#endif
#if TEST_FUNC
    testBPTreeFunction();
#endif
#if TEST_SPEED
    for (int i = 10000000; i >= 10000; i /= 10) {
        testBPTreeSpeed(i);
    }
#endif
#if TEST_MEM
    testBPTreeMemory();
#endif
#if TEST_SERIAL
    for (int i = 10000000;; i /= 10) {
        testBPTreeSerial(i);
        if (!i) {
            break;
        }
    }
#endif
    return 0;
}
