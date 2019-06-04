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

    runBlock([&]() {
        for (int i = 0; i < TEST_LIST_COUNT; ++i) {
            testTimeList.add(i);
        }
    }, "list push_back");

    testTimeList.clear();
    testTimeList.trimToSize();
    testTimeList.reserve(TEST_LIST_COUNT);
    runBlock([&]() {
        for (int i = 0; i < TEST_LIST_COUNT; ++i) {
            testTimeList.add(i);
        }
    }, "list(preserve) push_back");

    runBlock([&]() {
        for (int i = 0; i < TEST_LIST_COUNT; ++i) {
            v.push_back(i);
        }
    }, "vector push_back");

    for (int i = 0, size = testTimeList.getSize(); i < size; i++) {
        assert(v[i] == testTimeList.get(i));
    }

    v.clear();
    v.shrink_to_fit();
    v.reserve(TEST_LIST_COUNT);
    runBlock([&]() {
        for (int i = 0; i < TEST_LIST_COUNT; ++i) {
            v.push_back(i);
        }
    }, "vector(preserve) push_back");

    for (int i = 0, size = testTimeList.getSize(); i < size; i++) {
        assert(v[i] == testTimeList.get(i));
    }

    testTimeList.clear();
    testTimeList.trimToSize();
    runBlock([&]() {
        for (int i = 0; i < TEST_LIST_COUNT; ++i) {
            testTimeList.insert(0, i);
        }
    }, "list push_front");

    v.clear();
    v.shrink_to_fit();
    runBlock([&]() {
        for (int i = 0; i < TEST_LIST_COUNT; ++i) {
            v.insert(v.begin(), i);
        }
    }, "vector push_front");

    for (int i = 0, size = testTimeList.getSize(); i < size; i++) {
        assert(v[i] == testTimeList.get(i));
    }

    runBlock([&]() {
        for (int i = 0; i < TEST_LIST_COUNT / 2; ++i) {
            testTimeList.removeAt(0);
        }
    }, "list(front) remove half");

    runBlock([&]() {
        for (int i = 0; i < TEST_LIST_COUNT / 2; ++i) {
            v.erase(v.begin());
        }
    }, "vector(front) remove half");

    assert(testTimeList.getSize() == v.size());
    for (int i = 0, size = testTimeList.getSize(); i < size; i++) {
        assert(v[i] == testTimeList.get(i));
    }


    runBlock([&]() {
        for (int i = 0; i < TEST_LIST_COUNT / 2; ++i) {
            testTimeList.removeAt(testTimeList.getSize() - 1);
        }
    }, "list(back) remove half");

    runBlock([&]() {
        for (int i = 0; i < TEST_LIST_COUNT / 2; ++i) {
            v.erase(v.begin() + v.size() - 1);
        }
    }, "vector(back) remove half");

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


    runBlock([&]() {
        testTimeList.removeRange(0, TEST_LIST_COUNT / 2);
    }, "list remove range half");

    runBlock([&]() {
        v.erase(v.begin(), v.begin() + TEST_LIST_COUNT / 2);
    }, "vector remove range half");

    for (int i = 0, size = testTimeList.getSize(); i < size; i++) {
        assert(v[i] == testTimeList.get(i));
    }
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

    for (int i = 0; i < TEST_BP_TREE_FUNCTION_COUNT; ++i) {
        tree.put(arr[i], i);
    }

    for (int i = 0; i < TEST_BP_TREE_FUNCTION_COUNT; ++i) {
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


void testBPTreeSpeed(int testSpeedCount) {
    printf("***Start test speed. Count=%d\n", testSpeedCount);
    srand(static_cast<unsigned int>(time(nullptr)));
    BPTree<int, int> tree(static_cast<int>(pow(log(testSpeedCount), 2)));
    unique_ptr<int[]> testKey(new int[testSpeedCount]);
    unique_ptr<int[]> testValue(new int[testSpeedCount]);
    for (int i = 0; i < testSpeedCount; ++i) {
        testKey.get()[i] = rand();
        testValue.get()[i] = rand();
    }


    runBlock([&]() {
        for (int i = 0; i < testSpeedCount; ++i) {
            tree.put(testKey[i], testValue[i]);
        }
    }, "bp tree insert");

    runBlock([&]() {
        for (int i = 0; i < testSpeedCount; ++i) {
            tree.get(testKey[i]);
        }
    }, "bp tree access");

    runBlock([&]() {
        for (int i = 0; i < testSpeedCount; ++i) {
            tree.remove(testKey[i]);
        }
    }, "bp tree remove");


    map<int, int> m;
    runBlock([&]() {
        for (int i = 0; i < testSpeedCount; ++i) {
            m[testKey[i]] = testValue[i];
        }
    }, "stl map insert");

    runBlock([&]() {
        for (int i = 0; i < testSpeedCount; ++i) {
            m.at(testKey[i]);
        }
    }, "stl map access");

    runBlock([&]() {
        for (int i = 0; i < testSpeedCount; ++i) {
            m.erase(testKey[i]);
        }
    }, "stl map remove");

    printf("***End test speed.\n");
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

    for (int i = 1; i <= 5; ++i) {
        printf("start put(%d)...\n", i);
        for (int i = 0; i < TEST_BP_TREE_COUNT; ++i) {
            tree.put(i, i);
        }
        printf("start clear(%d)...\n", i);
        tree.clear();
    }
}


void testBPTreeSerial(int serialCount) {
    printf("**Start serial test. Count = %d\n", serialCount);
    srand(static_cast<unsigned int>(time(nullptr)));
    int *arr = new int[serialCount];
    for (int i = 0; i < serialCount; ++i) {
        arr[i] = i;
    }
    qsort(arr, static_cast<size_t>(serialCount), sizeof(int), randomComp);

    BPTree<int, int> tree(20);
    for (int i = 0; i < serialCount; ++i) {
        tree.put(arr[i], i);
    }
    string s = string("test.bpt");

    runBlock([&tree, &s]() {
        tree.serialize(s);
    }, "serialize");

    shared_ptr<BPTree<int, int>> dTreePtr;
    runBlock([&dTreePtr, &s]() {
        dTreePtr = BPTree<int, int>::deserialize(s);
    }, "deserialize");

    FILE *f = fopen("test.bpt", "r");
    if (!f) {
        fprintf(stderr, "cannot get serialized file size\n");
    }
    fseek(f, 0, SEEK_END);
    printf("serialize file size: %ld bytes\n", ftell(f));
    fclose(f);

    for (int i = 0; i < serialCount; ++i) {
        assert(*(dTreePtr->get(arr[i])) == i);
    }
    remove("test.bpt");
    printf("**End serial test.\n");
}

template<class Function>
void runBlock(Function func, const char *msg) {
    clock_t start = clock();
    func();
    clock_t end = clock();
    printf("%s use: %f ms\n", msg, 1000.0 * (end - start) / CLOCKS_PER_SEC);
}
