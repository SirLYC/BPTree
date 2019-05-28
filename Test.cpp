//
// Created by 刘育氚 on 2019/5/28.
//

#include "Test.h"
#include "List.h"
#include <vector>

#define TEST_LIST_COUNT 1000000

using namespace std;

List<int> list;

void testList() {
    srand(static_cast<unsigned int>(time(NULL)));

    List<int> l;

    l.add(1);
    assert(l.get(0) == 1);
    l.add(2);
    l.add(3);
    l.add(4);
    assert(l.get(2) == 3);
    l[2] = 6;
    assert(l.get(2) == 6);
    l.removeAt(0);
    assert(l.get(0) == 2);
    assert(l.get(1) == 6);
    assert(l.get(2) == 4);


    int s = l.getSize();
    List<int> l2;

    for (int i = 0; i < 100; i++) {
        l2.add(i);
    }
    l.add(l2);

    for (int j = 0, p = s; j < 100; ++j, ++p) {
        assert(l.get(p) == l2.get(j));
    }

    List<int> testTimeList;
    vector<int> v;

    clock_t start = clock();
    for (int i = 0; i < TEST_LIST_COUNT; ++i) {
        testTimeList.add(i);
    }
    clock_t end = clock();
    printf("list push_back use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    testTimeList.clear();
    testTimeList.trimToSize();
    testTimeList.reserve(TEST_LIST_COUNT);
    start = clock();
    for (int i = 0; i < TEST_LIST_COUNT; ++i) {
        testTimeList.add(i);
    }
    end = clock();
    printf("list(preserve) push_back use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < TEST_LIST_COUNT; ++i) {
        v.push_back(i);
    }
    end = clock();
    printf("vector push_back use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    for (int i = 0, size = testTimeList.getSize(); i < size; i++) {
        assert(v[i] == testTimeList.get(i));
    }

    v.clear();
    v.shrink_to_fit();
    v.reserve(TEST_LIST_COUNT);
    start = clock();
    for (int i = 0; i < TEST_LIST_COUNT; ++i) {
        v.push_back(i);
    }
    end = clock();
    printf("vector(preserve) push_back use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    for (int i = 0, size = testTimeList.getSize(); i < size; i++) {
        assert(v[i] == testTimeList.get(i));
    }

    testTimeList.clear();
    testTimeList.trimToSize();
    start = clock();
    for (int i = 0; i < TEST_LIST_COUNT; ++i) {
        testTimeList.insert(0, i);
    }
    end = clock();
    printf("list push_front use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    v.clear();
    v.shrink_to_fit();
    start = clock();
    for (int i = 0; i < TEST_LIST_COUNT; ++i) {
        v.insert(v.begin(), i);
    }
    end = clock();
    printf("vector push_front use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    for (int i = 0, size = testTimeList.getSize(); i < size; i++) {
        assert(v[i] == testTimeList.get(i));
    }

    start = clock();
    for (int i = 0; i < TEST_LIST_COUNT / 3; ++i) {
        testTimeList.removeAt(0);
    }
    end = clock();
    printf("list(front) remove half use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < TEST_LIST_COUNT / 3; ++i) {
        v.erase(v.begin());
    }
    end = clock();
    printf("vector(front) remove half use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    assert(testTimeList.getSize() == v.size());
    for (int i = 0, size = testTimeList.getSize(); i < size; i++) {
        assert(v[i] == testTimeList.get(i));
    }

    start = clock();
    for (int i = 0; i < TEST_LIST_COUNT / 3; ++i) {
        testTimeList.removeAt(testTimeList.getSize() - 1);
    }
    end = clock();
    printf("list(back) remove half use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < TEST_LIST_COUNT / 3; ++i) {
        v.erase(v.begin() + v.size() - 1);
    }
    end = clock();
    printf("vector(back) remove half use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    // fill data
    testTimeList.clear();
    testTimeList.trimToSize();
    for (int i = 0; i < TEST_LIST_COUNT; ++i) {
        testTimeList.insert(0, i);
    }
    v.clear();
    v.shrink_to_fit();
    for (int i = 0; i < TEST_LIST_COUNT; ++i) {
        v.insert(v.begin(), i);
    }

    start = clock();
    testTimeList.removeRange(0, TEST_LIST_COUNT / 2);
    end = clock();
    printf("list remove range half use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    start = clock();
    v.erase(v.begin(), v.begin() + TEST_LIST_COUNT / 2);
    end = clock();
    printf("vector remove range half use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    for (int i = 0, size = testTimeList.getSize(); i < size; i++) {
        assert(v[i] == testTimeList.get(i));
    }
}

void runWithtime(Runnable runnable, char *msg) {
    clock_t start = clock();
    runnable();
    clock_t end = clock();
    printf("%s : %fms", msg, 1000.0 * (end - start) / CLOCKS_PER_SEC);
}
