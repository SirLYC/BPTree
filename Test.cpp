//
// Created by 刘育氚 on 2019/5/28.
//

#include "Test.h"
#include "List.h"
#include "BPTree.h"
#include <vector>
#include <cmath>
#include <map>

#define TEST_LIST_COUNT 100000
#define TEST_BP_TREE_FUNCTION_COUNT 100000
#define TEST_BP_TREE_COUNT 10000000

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

    for (int i = 0; i < 100; i++) {
        l2.add(i);
    }
    l.insert(20, l2);

    for (int j = 0, p = 20; j < 100; ++j, ++p) {
        assert(l.get(p) == l2.get(j));
    }

    List<int> lCopy = l;
    l.insert(100, l2, 0, 20);
    assert(lCopy.getSize() + 20 == l.getSize());
    for (int i = 0; i < 100; i++) {
        assert(l[i] == lCopy[i]);
    }

    for (int i = 0, p = 100; i < 20; i++, p++) {
        assert(l[p] == l2[i]);
    }

    for (int i = 120, p = 100, s = l.getSize(); i < s; i++, p++) {
        assert(lCopy[p] == l[i]);
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
    for (int i = 0; i < TEST_LIST_COUNT / 2; ++i) {
        testTimeList.removeAt(0);
    }
    end = clock();
    printf("list(front) remove half use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < TEST_LIST_COUNT / 2; ++i) {
        v.erase(v.begin());
    }
    end = clock();
    printf("vector(front) remove half use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    assert(testTimeList.getSize() == v.size());
    for (int i = 0, size = testTimeList.getSize(); i < size; i++) {
        assert(v[i] == testTimeList.get(i));
    }

    start = clock();
    for (int i = 0; i < TEST_LIST_COUNT / 2; ++i) {
        testTimeList.removeAt(testTimeList.getSize() - 1);
    }
    end = clock();
    printf("list(back) remove half use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < TEST_LIST_COUNT / 2; ++i) {
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

static int randomComp(const void *a, const void *b) {
    return (rand() & 1) ? 1 : -1;
}

int lastKey;
int *nums;

static bool foreachFunc(const int &key, const int &value) {
    assert(nums[value] == key);
    assert(key > lastKey);
    lastKey = key;
    return false;
}

static bool foreachReverseFunc(const int &key, const int &value) {
    assert(nums[value] == key);
    assert(key < lastKey);
    lastKey = key;
    return false;
}

static bool foreachIndexFunc(int index, const int &key, const int &value) {
    assert(nums[value] == key);
    assert(index == key);
    assert(key > lastKey);
    lastKey = key;
    return false;
}

static bool foreachIndexReverseFunc(int index, const int &key, const int &value) {
    assert(nums[value] == key);
    assert(index == key);
    assert(key < lastKey);
    lastKey = key;
    return false;
}

void testForeach(BPTree<int, int> &tree, int *n) {
    nums = n;
    lastKey = -1;
    tree.foreach(foreachFunc);
    lastKey = -1;
    tree.foreachIndex(foreachIndexFunc);
    lastKey = TEST_BP_TREE_FUNCTION_COUNT + 1;
    tree.foreachReverse(foreachReverseFunc);
    lastKey = TEST_BP_TREE_FUNCTION_COUNT + 1;
    tree.foreachIndexReverse(foreachIndexReverseFunc);
    nums = nullptr;
}

void testBPTreeFunction() {
    srand(static_cast<unsigned int>(time(nullptr)));
    int *arr = new int[TEST_BP_TREE_FUNCTION_COUNT];
    for (int i = 0; i < TEST_BP_TREE_FUNCTION_COUNT; ++i) {
        arr[i] = i;
    }
    qsort(arr, TEST_BP_TREE_FUNCTION_COUNT, sizeof(int), randomComp);

    int order = static_cast<int>(log(TEST_BP_TREE_FUNCTION_COUNT)) + 1;
    BPTree<int, int> tree(order);

    for (int i = 0; i < TEST_BP_TREE_FUNCTION_COUNT; ++i) {
        tree.put(arr[i], i);
    }

    for (int i = 0; i < TEST_BP_TREE_FUNCTION_COUNT; ++i) {
        assert(tree.containsKey(arr[i]));
        assert(*(tree.get(arr[i])) == i);
    }

    for (int i = 0; i < TEST_BP_TREE_FUNCTION_COUNT / 2; ++i) {
        tree.remove(arr[i]);
    }

    for (int i = 0; i < TEST_BP_TREE_FUNCTION_COUNT / 2; ++i) {
        assert(!tree.containsKey(arr[i]));
        assert(!tree.get(arr[i]));
    }

    for (int i = TEST_BP_TREE_FUNCTION_COUNT / 2 + 1; i < TEST_BP_TREE_FUNCTION_COUNT; ++i) {
        assert(tree.containsKey(arr[i]));
        assert(*(tree.get(arr[i])) == i);
    }

    tree.clear();

    for (int i = 0; i < TEST_BP_TREE_FUNCTION_COUNT; ++i) {
        assert(!tree.containsKey(arr[i]));
        assert(!tree.get(arr[i]));
    }

    for (int i = 0; i < TEST_BP_TREE_FUNCTION_COUNT; ++i) {
        tree.put(arr[i], i);
    }

    for (int i = 0; i < TEST_BP_TREE_FUNCTION_COUNT; ++i) {
        assert(tree.containsKey(arr[i]));
        assert(*(tree.get(arr[i])) == i);
    }

    testForeach(tree, arr);
    delete[](arr);

    printf("bp tree function test passed!\n");
}


void testBPTreeSpeed() {
    srand(static_cast<unsigned int>(time(nullptr)));


    BPTree<int, int> tree(1000);
    int *testKey = new int[TEST_BP_TREE_COUNT];
    int *textValue = new int[TEST_BP_TREE_COUNT];
    for (int i = 0; i < TEST_BP_TREE_COUNT; ++i) {
        testKey[i] = rand();
        textValue[i] = rand();
    }


    clock_t start = clock();
    for (int i = 0; i < TEST_BP_TREE_COUNT; ++i) {
        tree.put(testKey[i], textValue[i]);
    }
    clock_t end = clock();
    printf("bp tree insert use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < TEST_BP_TREE_COUNT; ++i) {
        tree.get(testKey[i]);
    }
    end = clock();
    printf("bp tree access use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < TEST_BP_TREE_COUNT; ++i) {
        tree.remove(testKey[i]);
    }
    end = clock();
    printf("bp tree remove use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    map<int, int> m;
    start = clock();
    for (int i = 0; i < TEST_BP_TREE_COUNT; ++i) {
        m[testKey[i]] = textValue[i];
    }
    end = clock();
    printf("stl map insert use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);


    start = clock();
    for (int i = 0; i < TEST_BP_TREE_COUNT; ++i) {
        m[testKey[i]];
    }
    end = clock();
    printf("stl map access use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < TEST_BP_TREE_COUNT; ++i) {
        m.erase(testKey[i]);
    }
    end = clock();
    printf("stl map remove use: %f ms\n", 1000.0 * (end - start) / CLOCKS_PER_SEC);

    delete[]testKey;
    delete[]textValue;
}

void testBPTreeMemory() {
    srand(static_cast<unsigned int>(time(nullptr)));
    BPTree<int, int> tree(1000);

    printf("start put...\n");
    for (int i = 0; i < TEST_BP_TREE_COUNT; ++i) {
        tree.put(i, i);
    }

    printf("start remove half...\n");
    for (int i = 0; i < TEST_BP_TREE_COUNT / 2; ++i) {
        tree.remove(i);
    }

    printf("start remove half...\n");
    for (int i = TEST_BP_TREE_COUNT / 2 + 1; i < TEST_BP_TREE_COUNT; ++i) {
        tree.remove(i);
    }

    printf("start put...\n");
    for (int i = 0; i < TEST_BP_TREE_COUNT; ++i) {
        tree.put(i, i);
    }

    printf("start clear...\n");
    tree.clear();

    printf("start put...\n");
    for (int i = 0; i < TEST_BP_TREE_COUNT; ++i) {
        tree.put(i, i);
    }

    printf("start clear...\n");
    tree.clear();
}
