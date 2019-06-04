//
// Created by 刘育氚 on 2019/5/27.
//

#ifndef BPTREE_TEST_H
#define BPTREE_TEST_H

#include <ctime>
#include <cstdarg>
#include <cstdio>


void testList();

void testBPTreeFunction();

void testBPTreeSpeed(int testSpeedCount);

void testBPTreeMemory();

void testBPTreeSerial(int serialCount);

template<class Function>
void runBlock(Function func, const char *msg);

class A {
public:
    int a;
};

struct compareA {
    int operator()(const A &x, const A &y) const {
        if (x.a > y.a) {
            return 1;
        } else if (x.a == y.a) {
            return 0;
        } else {
            return -1;
        }
    }
};

#endif //BPTREE_TEST_H
