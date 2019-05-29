#include <iostream>
#include "BPTree.h"
#include <map>

using namespace std;

int testBPTree();

int main() {
//    testList();
    testBPTree();
    printf("***main finished");
}

int testBPTree() {
    BPTree<int, int> t(3);
    for (int i = 0; i < 10; ++i) {
        t.put(i, i);
    }

    srand(static_cast<unsigned int>(time(nullptr)));
    BPTree<int, int> tree(500);
    int testCount = 10000000;
    int *testKey = new int[testCount];
    int *textValue = new int[testCount];
    for (int i = 0; i < testCount; ++i) {
        testKey[i] = rand();
        textValue[i] = rand();
    }


    clock_t start = clock();
    for (int i = 0; i < testCount; ++i) {
        tree.put(testKey[i], textValue[i]);
    }
    clock_t end = clock();
    cout << "bp tree insert use: " << 1000.0 * (end - start) / CLOCKS_PER_SEC << "ms" << endl;

    start = clock();
    for (int i = 0; i < testCount; ++i) {
        int *value = tree.get(testKey[i]);
//        assert(*value == textValue[i]);
    }
    end = clock();
    cout << "bp tree access use: " << 1000.0 * (end - start) / CLOCKS_PER_SEC << "ms" << endl;

    map<int, int> m;
    start = clock();
    for (int i = 0; i < testCount; ++i) {
        m[testKey[i]] = textValue[i];
    }
    end = clock();
    cout << "stl map insert use: " << 1000.0 * (end - start) / CLOCKS_PER_SEC << "ms" << endl;


    start = clock();
    for (int i = 0; i < testCount; ++i) {
        int value = m[testKey[i]];
    }
    end = clock();
    cout << "stl map access use: " << 1000.0 * (end - start) / CLOCKS_PER_SEC << "ms" << endl;

    return 0;
}