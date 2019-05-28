//
// Created by 刘育氚 on 2019/5/24.
//

#ifndef BPTREE_BPTREE_H
#define BPTREE_BPTREE_H

#include <algorithm>
#include <vector>
#include "Comp.h"

static const int minOrder = 2;

template<typename K, typename V>
class BPTree {
private:

    struct Node {
        Node *parentPtr = NULL;
        Node *next = NULL;

        int order;
        int initCap;
        int minLoad;
        bool leaf;
        std::vector<Node *> childNodePtrs;
        std::vector<V> values;
        std::vector<K> keys;

        Node(int order, int initCap, bool leaf) :
                order(order),
                initCap(initCap),
                minLoad((order + 1) / 2),
                leaf(leaf) {
            childNodePtrs.reserve(static_cast<unsigned long>(initCap));
            if (leaf) {
                values.reserve(static_cast<unsigned long>(initCap));
            } else {
                keys.reserve(static_cast<unsigned long>(initCap));
            }
        };

    };

    void putInner(const K &key, const V &value);

    int order;
    int size;
    comparator<K> comp;
    int initCap = 0;
    Node *root;

    Node *getNodeByKey(K key);

    inline int compare(const K &x, const K &y) {
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

    int findKeyPosition(const std::vector<K> &keys, const K &val) {
        if (keys.empty()) {
            return 0;
        }

        int s = static_cast<int>(keys.size());
        int high = s;
        int low = 0;
        int mid;
        int toIndex = -1;
        int c;

        while (low < high) {
            mid = (low + high) >> 1;
            c = compare(val, keys[mid]);
            if (c == 0) {
                toIndex = mid;
                break;
            } else if (c > 0) {
                low = mid + 1;
            } else {
                high = mid;
            }
        }

        if (toIndex == -1) {
            toIndex = low;
        }
        return toIndex;
    }

    bool putToNode(Node *node, const K &key, const V *value, Node *insertNode);

    Node *split(Node *nodePtr) {
        std::vector<K> &keys = nodePtr->keys;
        if (keys.size() <= order) {
            return NULL;
        }

        int mid = (static_cast<int>(keys.size()) + 1) / 2;
        Node *parentPtr = nodePtr->parentPtr;
        if (!parentPtr) {
            parentPtr = new Node(order, initCap, false);
            nodePtr->parentPtr = parentPtr;
        }

        auto keyBegin = keys.begin();
        // split
        Node *left = new Node(order, initCap, nodePtr->leaf);
        Node *right = new Node(order, initCap, nodePtr->leaf);
        std::vector<K> &leftKeys = left->keys;
        std::vector<K> &rightKeys = right->keys;
        leftKeys.assign(keyBegin, keyBegin + mid);
        rightKeys.assign(keyBegin + mid, keys.end());
        left->parentPtr = parentPtr;
        right->parentPtr = parentPtr;
        if (nodePtr->leaf) {
            auto valueBegin = nodePtr->values.begin();
            left->values.assign(valueBegin, valueBegin + mid);
            right->values.assign(valueBegin + mid, nodePtr->values.end());
            left->next = right;
        } else {
            auto nodeBegin = nodePtr->childNodePtrs.begin();
            left->childNodePtrs.assign(nodeBegin, nodeBegin + mid);
            right->childNodePtrs.assign(nodeBegin + mid, nodePtr->childNodePtrs.end());
            for (Node *&n: left->childNodePtrs) {
                n->parentPtr = left;
            }
            for (Node *&n: right->childNodePtrs) {
                n->parentPtr = right;
            }
        }
        putToNode(parentPtr, leftKeys[leftKeys.size() - 1], NULL, left);
        putToNode(parentPtr, rightKeys[rightKeys.size() - 1], NULL, right);
        return parentPtr;
    }

public:

    explicit BPTree(int orders) : BPTree(orders, 0, NULL) {}

    BPTree(int order, comparator<K> comp) : BPTree(order, 0, comp) {}

    explicit BPTree(int order, int initCap, comparator<K> comp) :
            order(std::max(minOrder, order)),
            size(0),
            initCap(std::max(std::min(this->order,
                                      initCap), (this->order + 1) / 2)),
            comp(comp),
            root(new Node(this->order,
                          this->initCap,
                          true)) {}

    // interface
    void put(const K &key, const V &value);

    V *remove(K key);

    V *get(K key);

    int getOrder();

    int getSize();
};


template<typename K, typename V>
void BPTree<K, V>::put(const K &key, const V &value) {
    putInner(key, value);
}

template<typename K, typename V>
V *BPTree<K, V>::remove(K key) {
    return NULL;
}

template<typename K, typename V>
V *BPTree<K, V>::get(K key) {
    Node *curPtr = root;
    int pos;
    while (!curPtr->leaf) {
        int s = static_cast<int>(curPtr->childNodePtrs.size());
        const std::vector<K> constKeys = curPtr->keys;
        pos = findKeyPosition(constKeys, key);
        if (pos == s) {
            pos--;
        }
        curPtr = curPtr->childNodePtrs[pos];
    }
    const std::vector<K> constKeys = curPtr->keys;
    pos = findKeyPosition(constKeys, key);
    if (pos >= 0 && pos < curPtr->values.size() && compare(curPtr->keys[pos], key) == 0) {
        return &(curPtr->values[pos]);
    }

    return NULL;
}

template<typename K, typename V>
typename BPTree<K, V>::Node *BPTree<K, V>::getNodeByKey(K key) {
    return nullptr;
}

template<typename K, typename V>
int BPTree<K, V>::getOrder() {
    return order;
}

template<typename K, typename V>
int BPTree<K, V>::getSize() {
    return size;
}

template<typename K, typename V>
void BPTree<K, V>::putInner(const K &key, const V &value) {
    bool result;
    Node *insertNode = root;
    int pos = -1;
    while (!insertNode->leaf) {
        int s = static_cast<int>(insertNode->childNodePtrs.size());
        const std::vector<K> constKeys = insertNode->keys;
        pos = findKeyPosition(constKeys, key);
        if (pos == s) {
            pos--;
        }
        insertNode = insertNode->childNodePtrs[pos];
    }

    result = putToNode(insertNode, key, &value, NULL);
    if (result) size++;

    Node *tmp = insertNode->parentPtr;
    while (tmp) {
        std::vector<K> &keys = tmp->keys;
        K oldKey = keys[pos];
        if (pos >= 0 && pos < keys.size() && compare(oldKey, key) < 0) {
            keys[pos] = key;
            if (pos == keys.size() - 1 && (tmp = tmp->parentPtr)) {
                pos = findKeyPosition(tmp->keys, oldKey);
                continue;
            }
        }
        break;
    }

    int s = static_cast<int>(insertNode->keys.size());
    if (result && s > order) {
        Node *node = insertNode;
        Node *splitParent;
        Node *p;
        Node *deletePtr;
        do {
            p = node->parentPtr;
            splitParent = split(node);
            deletePtr = node;
            delete (deletePtr);
            if (splitParent != p) {
                this->root = splitParent;
                break;
            } else if (splitParent->keys.size() > order) {
                node = splitParent;
            } else {
                break;
            }
        } while (true);
    }
}

template<typename K, typename V>
bool

BPTree<K, V>::putToNode(BPTree::Node *nodePtr, const K &key, const V *value, BPTree::Node *insertNodePtr) {
    int toIndex = findKeyPosition(nodePtr->keys, key);
    bool present = toIndex < nodePtr->keys.size() && compare(key, nodePtr->keys[toIndex]) == 0;

    if (present) {
        if (nodePtr->leaf) {
            nodePtr->values[toIndex] = *value;
        } else {
            nodePtr->childNodePtrs[toIndex] = insertNodePtr;
        }
        return false;
    } else {
        nodePtr->keys.insert(nodePtr->keys.begin() + toIndex, key);
        if (nodePtr->leaf) {
            nodePtr->values.insert(nodePtr->values.begin() + toIndex, *value);
        } else {
            nodePtr->childNodePtrs.insert(nodePtr->childNodePtrs.begin() + toIndex, insertNodePtr);
        }
    }

    return true;
}


#endif //BPTREE_BPTREE_H
