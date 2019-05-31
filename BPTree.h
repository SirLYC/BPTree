//
// Created by 刘育氚 on 2019/5/24.
//

#ifndef BPTREE_BPTREE_H
#define BPTREE_BPTREE_H

#include <algorithm>
#include "Comp.h"
#include "List.h"

#define MIN_ORDER 2

template<typename K, typename V> using biApply = bool (*)(const K &, const V &);
template<typename K, typename V> using biApplyIndex = bool (*)(int index, const K &, const V &);

template<typename K, typename V>
class BPTree {
private:

    struct Node {
        Node *parentPtr = NULL;
        Node *previous = NULL;
        Node *next = NULL;

        int order;
        int initCap;
        bool leaf;
        List<Node *> childNodePtrs;
        List<V> values;
        List<K> keys;

        Node(int order, int initCap, bool leaf) :
                order(order),
                initCap(initCap),
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
    int minLoad;
    comparator<K> comp;
    int initCap = 0;
    Node *root;

    bool putToNode(Node *nodePtr, const K &key, const V *value, Node *insertNodePtr);

    int deleteFromNode(Node *nodePtr, const K &key);

    void updateParentKey(Node *node, K &oldKey, K &newKey, int pos);

    Node *fixNode(Node *nodePtr);

    Node *split(Node *nodePtr);

    Node *getFirstLeaf();

    Node *getLastLeaf();

    void clear(Node *node);

public:

    explicit BPTree(int orders) : BPTree(orders, 0, NULL) {}

    BPTree(int order, comparator<K> comp) : BPTree(order, 0, comp) {}

    explicit BPTree(int order, int initCap, comparator<K> comp) :
            order(std::max(MIN_ORDER, order)),
            size(0),
            minLoad((this->order + 1) / 2),
            initCap(std::max(std::min(this->order,
                                      initCap), this->minLoad)),
            comp(comp),
            root(new Node(this->order,
                          this->initCap,
                          true)) {}

    ~BPTree() {
        clear(root);
        delete root;
    }

    // interface
    void put(const K &key, const V &value);

    void remove(K &key);

    V *get(K key);

    bool containsKey(const K &key);

    int getOrder();

    int getSize();

    void foreach(biApply<K, V> func);

    void foreachReverse(biApply<K, V> func);

    void foreachIndex(biApplyIndex<K, V> func);

    void foreachIndexReverse(biApplyIndex<K, V> func);

    void clear();
};


template<typename K, typename V>
void BPTree<K, V>::put(const K &key, const V &value) {
    putInner(key, value);
}

template<typename K, typename V>
void BPTree<K, V>::remove(K &key) {
    Node *deleteNodePtr = root;
    int pos = -1;
    while (!deleteNodePtr->leaf) {
        int s = deleteNodePtr->childNodePtrs.getSize();
        List<K> constKeys = deleteNodePtr->keys;
        pos = constKeys.binaryFind(key);
        if (pos == s) {
            pos--;
        }
        deleteNodePtr = deleteNodePtr->childNodePtrs[pos];
    }

    int deleteKeyPos = deleteFromNode(deleteNodePtr, key);
    if (deleteKeyPos < 0 || deleteNodePtr == root) {
        return;
    }
    if (pos == -1) {
        pos = deleteKeyPos;
    }


    if (deleteKeyPos == deleteNodePtr->keys.getSize()) {
        updateParentKey(deleteNodePtr, key, deleteNodePtr->keys[deleteNodePtr->keys.getSize() - 1], pos);
    }

    Node *tmpNodePtr = deleteNodePtr;
    while ((tmpNodePtr = fixNode(tmpNodePtr)));
    if (root->childNodePtrs.getSize() == 1) {
        Node *oldRoot = root;
        root = root->childNodePtrs[0];
        root->parentPtr = NULL;
        delete oldRoot;
    }
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
    const List<K> &constKeys = curPtr->keys;
    pos = constKeys.binaryFind(key);
    if (pos >= 0 && pos < curPtr->values.getSize() && compare(curPtr->keys[pos], key, comp) == 0) {
        return &(curPtr->values[pos]);
    }

    return NULL;
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
        left->previous = nodePtr->previous;
        nodePtr->previous = left;
        left->next = nodePtr;
        if (left->previous) {
            left->previous->next = left;
        }
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

template<typename K, typename V>
int BPTree<K, V>::deleteFromNode(BPTree::Node *nodePtr, const K &key) {
    List<K> &keys = nodePtr->keys;
    int pos = keys.binaryFind(key);
    if (pos < 0 || pos >= nodePtr->keys.getSize() || keys[pos] != key) {
        return -1;
    }
    keys.removeAt(pos);
    if (nodePtr->leaf) {
        nodePtr->values.removeAt(pos);
    } else {
        Node *node = nodePtr->childNodePtrs.get(pos);
        nodePtr->childNodePtrs.removeAt(pos);
        delete node;
    }
    return pos;
}

template<typename K, typename V>
typename BPTree<K, V>::Node *BPTree<K, V>::fixNode(BPTree::Node *nodePtr) {
    if (minLoad <= nodePtr->keys.getSize() || !nodePtr->parentPtr) {
        return NULL;
    }

    List<K> &parentKeys = nodePtr->parentPtr->keys;
    int parentKeySize = parentKeys.getSize();
    K &key = nodePtr->keys[nodePtr->keys.getSize() - 1];
    int pos = nodePtr->parentPtr->keys.binaryFind(key);

    Node *sibling;
    // borrow from previous
    int previousIndex = pos - 1;
    bool hasPrevious = previousIndex >= 0 && previousIndex < parentKeySize;
    if (hasPrevious && (sibling = nodePtr->parentPtr->childNodePtrs[previousIndex])->keys.getSize() > minLoad) {
        int s = sibling->keys.getSize();
        K &oldKey = sibling->keys[s - 1];
        K &newKey = sibling->keys[s - 2];
        nodePtr->keys.insert(0, sibling->keys[s - 1]);

        if (nodePtr->leaf) {
            nodePtr->values.insert(0, sibling->values[s - 1]);
            sibling->values.removeAt(s - 1);
        } else {
            nodePtr->childNodePtrs.insert(0, sibling->childNodePtrs[s - 1]);
            sibling->childNodePtrs.get(s - 1)->parentPtr = nodePtr;
            sibling->childNodePtrs.removeAt(s - 1);
        }
        updateParentKey(sibling, oldKey, newKey, previousIndex);
        sibling->keys.removeAt(s - 1);
        return NULL;
    }

    // borrow from next
    int nextIndex = pos + 1;
    bool hasNext = nextIndex >= 0 && nextIndex < parentKeySize;
    if (hasNext && (sibling = nodePtr->parentPtr->childNodePtrs[nextIndex])->keys.getSize() > minLoad) {
        int s = nodePtr->keys.getSize();
        K &oldKey = nodePtr->keys[s - 1];
        K &newKey = sibling->keys[0];
        nodePtr->keys.add(newKey);

        if (nodePtr->leaf) {
            nodePtr->values.add(sibling->values[0]);
            sibling->values.removeAt(0);
        } else {
            nodePtr->childNodePtrs.add(sibling->childNodePtrs[0]);
            sibling->childNodePtrs.get(0)->parentPtr = nodePtr;
            sibling->childNodePtrs.removeAt(0);
        }
        updateParentKey(nodePtr, oldKey, newKey, pos);
        sibling->keys.removeAt(0);
        return NULL;
    }

    // merge with previous
    if (hasPrevious) {
        sibling = nodePtr->parentPtr->childNodePtrs[previousIndex];
        nodePtr->keys.insert(0, sibling->keys);
        if (nodePtr->leaf) {
            nodePtr->values.insert(0, sibling->values);
            if (sibling->previous) {
                sibling->previous->next = nodePtr;
            }
            nodePtr->previous = sibling->previous;
        } else {
            nodePtr->childNodePtrs.insert(0, sibling->childNodePtrs);
            int s = sibling->childNodePtrs.getSize();
            for (int i = 0; i < s; ++i) {
                sibling->childNodePtrs.get(i)->parentPtr = nodePtr;
            }
        }
        nodePtr->parentPtr->keys.removeAt(previousIndex);
        nodePtr->parentPtr->childNodePtrs.removeAt(previousIndex);
        delete sibling;
        return nodePtr->parentPtr;
    }

    if (hasNext) {
        sibling = nodePtr->parentPtr->childNodePtrs[nextIndex];
        sibling->keys.insert(0, nodePtr->keys);
        if (nodePtr->leaf) {
            sibling->values.insert(0, nodePtr->values);
            if (nodePtr->previous) {
                nodePtr->previous->next = sibling;
            }
            sibling->previous = nodePtr->previous;
        } else {
            sibling->childNodePtrs.insert(0, nodePtr->childNodePtrs);
            int s = nodePtr->childNodePtrs.getSize();
            for (int i = 0; i < s; ++i) {
                nodePtr->childNodePtrs.get(i)->parentPtr = sibling;
            }
        }
        sibling->parentPtr->keys.removeAt(pos);
        sibling->parentPtr->childNodePtrs.removeAt(pos);
        delete nodePtr;
        return sibling->parentPtr;
    }

    // parent is root
    if (nodePtr->parentPtr == root) {
        root->childNodePtrs.clear();
        root->values.add(nodePtr->values);
        root->keys.add(nodePtr->keys);
        return NULL;
    }

    return NULL;
}

template<typename K, typename V>
void BPTree<K, V>::updateParentKey(BPTree::Node *node, K &oldKey, K &newKey, int pos) {
    Node *tmpNodePtr = node;
    // delete old key, update parent
    while (tmpNodePtr && tmpNodePtr->parentPtr) {
        tmpNodePtr = tmpNodePtr->parentPtr;
        tmpNodePtr->keys[pos] = newKey;
        if (pos == tmpNodePtr->keys.getSize() - 1 && tmpNodePtr->parentPtr) {
            pos = tmpNodePtr->parentPtr->keys.binaryFind(oldKey);
            continue;
        }
        break;
    }
}

template<typename K, typename V>
void BPTree<K, V>::clear(BPTree::Node *node) {
    if (!node->leaf) {
        int s = node->childNodePtrs.getSize();
        for (int i = 0; i < s; ++i) {
            clear(node->childNodePtrs[i]);
        }
    }

    int s = node->childNodePtrs.getSize();
    for (int i = 0; i < s; ++i) {
        delete node->childNodePtrs[i];
    }
    node->childNodePtrs.clear();
    node->keys.clear();
    node->values.clear();
}

template<typename K, typename V>
void BPTree<K, V>::clear() {
    if (size == 0)
        return;
    clear(root);
    root->leaf = true;
    size = 0;
}

template<typename K, typename V>
bool BPTree<K, V>::containsKey(const K &key) {
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
    const List<K> &constKeys = curPtr->keys;
    pos = constKeys.binaryFind(key);
    if (pos >= 0 && pos < curPtr->values.getSize()) {
        return compare(curPtr->keys[pos], key, comp) == 0;
    }

    return false;
}

template<typename K, typename V>
void BPTree<K, V>::foreach(biApply<K, V> func) {
    Node *node = getFirstLeaf();
    while (node) {
        for (int i = 0, s = node->values.getSize(); i < s; ++i) {
            if (func(node->keys[i], node->values[i])) {
                return;
            }
        }
        node = node->next;
    }
}

template<typename K, typename V>
void BPTree<K, V>::foreachIndex(biApplyIndex<K, V> func) {
    Node *node = getFirstLeaf();
    int index = 0;
    while (node) {
        for (int i = 0, s = node->values.getSize(); i < s; ++i, index++) {
            if (func(index, node->keys[i], node->values[i])) {
                return;
            }
        }
        node = node->next;
    }
}

template<typename K, typename V>
typename BPTree<K, V>::Node *BPTree<K, V>::getFirstLeaf() {
    Node *node = root;
    while (!node->leaf) {
        node = node->childNodePtrs[0];
    }
    return node;
}


template<typename K, typename V>
typename BPTree<K, V>::Node *BPTree<K, V>::getLastLeaf() {
    Node *node = root;
    while (!node->leaf) {
        node = node->childNodePtrs[node->childNodePtrs.getSize() - 1];
    }
    return node;
}

template<typename K, typename V>
void BPTree<K, V>::foreachReverse(biApply<K, V> func) {
    Node *node = getLastLeaf();
    while (node) {
        for (int i = node->values.getSize() - 1; i >= 0; --i) {
            if (func(node->keys[i], node->values[i])) {
                return;
            }
        }
        node = node->previous;
    }
}

template<typename K, typename V>
void BPTree<K, V>::foreachIndexReverse(biApplyIndex<K, V> func) {
    Node *node = getLastLeaf();
    int index = size - 1;
    while (node) {
        for (int i = node->values.getSize() - 1; i >= 0; --i, --index) {
            if (func(index, node->keys[i], node->values[i])) {
                return;
            }
        }
        node = node->previous;
    }
}

#endif //BPTREE_BPTREE_H
