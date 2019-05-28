//
// Created by 刘育氚 on 2019/5/27.
//

#ifndef BPTREE_TEST_H
#define BPTREE_TEST_H

#include <ctime>
#include <cstdarg>
#include <cstdio>

using Runnable = void (*)();

void testList();

void runWithtime(Runnable runnable, char *msg);

#endif //BPTREE_TEST_H
