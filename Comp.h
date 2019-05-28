//
// Created by 刘育氚 on 2019/5/27.
//

#ifndef BPTREE_COMP_H
#define BPTREE_COMP_H

template<typename K>
// < 0: less
// == 0: equal
// > 0: greater
using comparator = int (*)(const K &x, const K &y);

template<typename K>
int compare(K &x, K &y, comparator<K> comp) {
    if (comp) {
        return comp(x, y);
    } else if (x > y) {
        return 1;
    } else if (x == y) {
        return 0;
    } else {
        return -1;
    }
}

#endif //BPTREE_COMP_H
