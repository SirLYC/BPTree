//
// Created by 刘育氚 on 2019/5/27.
//

#ifndef BPTREE_COMP_H
#define BPTREE_COMP_H

template<typename T>
struct DefaultCompare {
    int operator()(const T &x, const T &y) const {
        if (x > y) {
            return 1;
        } else if (x == y) {
            return 0;
        } else {
            return -1;
        }
    }
};
#endif //BPTREE_UTILS_H
