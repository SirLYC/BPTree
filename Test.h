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

#endif //BPTREE_TEST_H
