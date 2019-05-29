//
// Created by 刘育氚 on 2019/5/24.
//

#ifndef BPTREE_BPTREE_H
#define BPTREE_BPTREE_H

#include <algorithm>
#include "Comp.h"
#include "List.h"

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
        List<Node *> childNodePtrs;
        List<V> values;
        List<K> keys;

        Node(int order, int initCap, bool leaf) :
                order(order),
                initCap(initCap),
                minLoad((order + 1) / 2),
                leaf(leaf) {
            childNodePtrs.reserve(initCap);
            if (leaf) {
                values.reserve(initCap);
            } else {
                keys.reserve(initCap);
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

    bool putToNode(Node *nodePtr, const K &key, const V *value, Node *insertNodePtr);

    Node *split(Node *nodePtr);

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
        int s = curPtr->childNodePtrs.getSize();
        List<K> constKeys = curPtr->keys;
        pos = constKeys.binaryFind(key);
        if (pos == s) {
            pos--;
        }
        curPtr = curPtr->childNodePtrs[pos];
    }
    const List<K> constKeys = curPtr->keys;
    pos = constKeys.binaryFind(key);
    if (pos >= 0 && pos < curPtr->values.getSize() && compare(curPtr->keys[pos], key, comp) == 0) {
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
        int s = insertNode->childNodePtrs.getSize();
        List<K> constKeys = insertNode->keys;
        pos = constKeys.binaryFind(key);
        if (pos == s) {
            pos--;
        }
        insertNode = insertNode->childNodePtrs[pos];
    }

    result = putToNode(insertNode, key, &value, NULL);
    if (result) size++;

    Node *tmp = insertNode->parentPtr;
    while (tmp) {
        List<K> &keys = tmp->keys;
        K oldKey = keys[pos];
        if (pos >= 0 && pos < keys.getSize() && compare(oldKey, key, comp) < 0) {
            keys[pos] = key;
            if (pos == keys.getSize() - 1 && (tmp = tmp->parentPtr)) {
                pos = tmp->keys.binaryFind(oldKey);
                continue;
            }
        }
        break;
    }

    int s = insertNode->keys.getSize();
    if (result && s > order) {
        Node *node = insertNode;
        Node *splitParent;
        Node *p;
        do {
            p = node->parentPtr;
            splitParent = split(node);
            if (splitParent != p) {
                this->root = splitParent;
                break;
            } else if (splitParent->keys.getSize() > order) {
                node = splitParent;
            } else {
                break;
            }
        } while (true);
    }
}

template<typename K, typename V>
bool BPTree<K, V>::putToNode(BPTree::Node *nodePtr, const K &key, const V *value, BPTree::Node *insertNodePtr) {
    int toIndex = nodePtr->keys.binaryFind(key);
    bool present = toIndex < nodePtr->keys.getSize() && compare(key, nodePtr->keys[toIndex], comp) == 0;

    if (present) {
        if (nodePtr->leaf) {
            nodePtr->values[toIndex] = *value;
        } else {
            nodePtr->childNodePtrs[toIndex] = insertNodePtr;
        }
        return false;
    } else {
        nodePtr->keys.insert(toIndex, key);
        if (nodePtr->leaf) {
            nodePtr->values.insert(toIndex, *value);
        } else {
            nodePtr->childNodePtrs.insert(toIndex, insertNodePtr);
        }
    }

    return true;
}

template<typename K, typename V>
typename BPTree<K, V>::Node *BPTree<K, V>::split(BPTree::Node *nodePtr) {
    List<K> &keys = nodePtr->keys;
    if (keys.getSize() <= order) {
        return NULL;
    }

    int mid = (keys.getSize() + 1) / 2;
    Node *parentPtr = nodePtr->parentPtr;
    if (!parentPtr) {
        parentPtr = new Node(order, initCap, false);
        nodePtr->parentPtr = parentPtr;
    }

    // split
    Node *left = new Node(order, initCap, nodePtr->leaf);
    List<K> &leftKeys = left->keys;
    List<K> &rightKeys = nodePtr->keys;
    leftKeys.add(rightKeys, 0, mid);
    rightKeys.removeRange(0, mid);
    left->parentPtr = parentPtr;
    if (nodePtr->leaf) {
        left->values.add(nodePtr->values, 0, mid);
        nodePtr->values.removeRange(0, mid);
        left->next = nodePtr;
    } else {
        left->childNodePtrs.add(nodePtr->childNodePtrs, 0, mid);
        nodePtr->childNodePtrs.removeRange(0, mid);
        for (int i = 0, s = left->childNodePtrs.getSize(); i < s; ++i) {
            Node *&n = left->childNodePtrs[i];
            n->parentPtr = left;
        }
    }
    putToNode(parentPtr, leftKeys[leftKeys.getSize() - 1], NULL, left);
    putToNode(parentPtr, rightKeys[rightKeys.getSize() - 1], NULL, nodePtr);
    return parentPtr;
}


#endif //BPTREE_BPTREE_H
